# Phase 2-04：Sensor + OLED — 加速度與陀螺儀即時顯示

## 📊 測試總覽

| 項目 | 內容 |
| --- | --- |
| 目的 | 把 2-03 換算好的加速度、陀螺儀數值即時顯示在 SSD1306 OLED 上，UART 同步輸出對照；確認 I²C 感測器和 SPI 顯示器在同一個迴圈裡不互相干擾，並量出 OLED 更新一次要多久 |
| 硬體 | OLED 沿用 [Phase 2-01](../01_SPI_Optimization/01_README.md) 的接線（PA_7 MOSI / PA_5 SCLK / PB_10 CS / PB_1 DC / PB_0 RES），SPI 的 MISO 指定 PA_6（D12 不接線）；IMU 沿用 [Phase 2-02](../02_I2C_LSM6DS3/02_README.md) 的接線（I2C1：PB8 = SCL、PB9 = SDA）；兩個模組透過麵包板電源軌共用 3.3V 和 GND |
| 流程 | 開機：OLED 初始化（1MHz）→ 切到 8MHz 設定（實際 5.63MHz）→ 清屏 → 設定 IMU → 畫標題。之後每 100 ms：等資料就緒 → 讀 12 bytes → UART 印一行 → 重畫 X／Y／Z 三行和計數器 |
| 顯示格式 | 加速度用整數 mg、陀螺儀用 dps 一位小數；每次整行重畫，數字變短時不殘留舊字 |
| 輸出 | `printf` 走 ST-LINK 虛擬 COM port，115200 baud |
| 量測 | Saleae Logic 2，24 MS/s；D0／D1 = SCL／SDA，D2 = SCLK，D3 = CS，D4 = MOSI，D5 = DC |

OLED 畫面：

```
ACC[mg]   GYR[dps]
X    31   X   2.2
Y   -61   Y  -2.6
Z  1019   Z  -0.3

n=123
```

## 🔍 結果

### 功能

| 測試 | 預期 | 實測 |
| --- | --- | --- |
| 開機 | UART 印出 `config OK`，OLED 出現標題 | ✅ RESET 5 次都正常 |
| OLED 與 UART 一致 | 同一筆資料的 X／Y／Z 相同（陀螺儀為 mdps ÷ 1000、取一位小數） | ✅ OLED 676／412／643、2.9／−3.0／−0.3，與 UART 同一筆完全相同 |
| 六面擺放 | 朝上那一軸約 ±1000 mg | ✅ 見下表 |
| 數字變短不殘留 | 從 ±1000 翻到接近 0 時，畫面沒有殘留的字 | ✅ |
| 計數器 | n 持續增加 | ✅ |

### 六面擺放（OLED 讀值）

| 朝上 | X [mg] | Y [mg] | Z [mg] |
| --- | --- | --- | --- |
| +Z | −1 | 2 | **1021** |
| −Z | −60 | 24 | **−981** |
| +X | **1012** | −12 | −17 |
| −X | **−993** | −14 | 25 |
| +Y | 292 | **952** | 62 |
| −Y | 41 | **−1005** | −29 |

- +Y 時模組傾斜約 17°，三軸合起來的大小約 998 mg

### 時序（[`captures/20261031_timing.sal`](captures/20261031_timing.sal)，28 個迴圈）

| 項目 | 預估 | 實測 |
| --- | --- | --- |
| I²C 讀取（4 個 START） | 約 1.7 ms | 1.792 ms |
| OLED 更新（828 byte） | 約 9 ms | **6.966 ms** |
| 迴圈週期 | 約 111 ms | 108.007 ms |

- 28 個迴圈之間的差異都在 0.005 ms 以內
- **預估怎麼算**：目前的驅動每畫一個字要送 12 個 byte（6 個重設游標的指令、6 個像素資料），三行加計數器共 69 個字 = 828 byte
- OLED 比預估快：預估用的是 2-01 單線模式的每 byte 11.0 µs，改成雙線後 8MHz 實測每 byte 8.0 µs

## 🐛 問題與根因

| # | 現象 | 根因 | 解法 |
| --- | --- | --- | --- |
| ① | OLED 沒有正常顯示 | 接線圖的 RES 和 CS 畫反，實際接線也照著接反 | 兩條線換回來 |
| ② | UART 印出 `config FAIL`，連 2-03 的程式也失敗 | IMU 的 GND 沒接。LA 看到 START 時 SCL 只低了 958ns 就彈回，第一筆位址解成 `[0x6D] Read + NAK` | 接上 GND：SDA 先降、5.053µs 後 SCL 才降（UM10204：≥ 4.0µs），位址 `[0x6B] Write + ACK` |
| ③ | — | OLED 的 GND 線插在麵包板的空孔，沒有接到地（拔掉 LA 後量導通才發現） | 補接 GND |
| ④ | 不接邏輯分析儀時 OLED 不顯示，或跑一陣子後錯位、亂點、變暗；Day 6 的程式只接 OLED 也一樣 | MISO = NC 時 mbed 用單線模式，每個 byte 開頭 SCLK 多一根 ≤ 42ns 的窄脈衝（2-01 根因 ①）。SSD1306 在 SCLK 上升緣讀 MOSI，整個 byte 往右位移 1 格，收到 `0x80 \| (原本的值 >> 1)` | MISO 指定 PA_6（D12 不接線），改成雙線模式；`oled_ssd1306.cpp` 與 2-01 目前的版本相同 |

### ④ 現象與位移後指令的對照

| 現象 | 程式送出 → OLED 收到 | SSD1306 的解讀 |
| --- | --- | --- |
| 不顯示（按 RESET 也不行） | `0xAF` → `0xD7`；`0x8D 0x14` → `0xC6 0x8A` | 沒有開顯示、電荷泵沒開 |
| 錯位 | `0xA6` → `0xD3`；`0x21`／`0x22` → `0x90`／`0x91` | `0xD3` 是設定顯示偏移；游標範圍沒設成功，字畫到錯的位置 |
| 變暗 | 頁位址 `0x02`／`0x03` → `0x81` | 設定對比，下一個 byte 被當成對比值（X、Y 剛好畫在 page 2、3） |
| 亂點 | 像素資料（DC = 高）位移 | 像素錯 |

- 對照表是用 shift1 公式對程式送出的 byte 推算的；不接 LA 才會出問題，所以無法逐 byte 擷取確認

| 排查時試過 | 結果 |
| --- | --- |
| GPIO 速度從 very high 降到 medium | 無效 |
| SPI 降到 1MHz | 無效 |
| OLED 電源加 104 電容 | 無效 |
| SCLK 加 100pF 取代探針 | 能顯示，但 n = 60 幾開始錯位 |
| 改成雙線模式 | ✅ 不接 LA、8MHz，RESET 5 次都正常，沒有錯位、亂點、反白或變暗 |

## ✅ 結論

- I²C 感測器和 SPI 顯示器在同一個迴圈裡運作沒有互相干擾：28 個迴圈的時序都一樣，六面擺放和 UART 對照也都正確
- OLED 更新一次 6.97 ms，約佔迴圈的 6%；作為之後優化的比較基準
- 這次遇到的問題都出在接線和 SPI 模式：RES／CS 對調、兩個模組的 GND 沒接好、MISO = NC 造成單線模式

## 🔬 時序分析工具 `tools/timing.py`

讀 Logic 2 匯出的原始資料 CSV（欄位順序 Time, SCL, SDA, SCLK, CS, MOSI, DC），以閒置超過 20 ms 切成一個個迴圈，算出每個迴圈的 I²C 時間、START 數、OLED 更新時間、SPI byte 數與迴圈週期。

```powershell
python Phase2_AttitudeSensing/04_Sensor_OLED/tools/timing.py <raw.csv>
```

- 原始資料 CSV 約 11MB，沒有放進 repo；用 Logic 2 開 `.sal` → File → Export Raw Data → CSV 即可重新產生

## 📁 檔案結構

```
Phase2_AttitudeSensing/04_Sensor_OLED/
├── 04_main.cpp              # 初始化 → 等資料就緒 → 讀 6 軸 → UART + OLED 重畫
├── Core/
│   ├── Inc/
│   │   ├── lsm6ds3.h        # 沿用 2-03，沒有修改
│   │   └── oled_ssd1306.h
│   └── Src/
│       ├── lsm6ds3.cpp
│       └── oled_ssd1306.cpp # 與 2-01 相同：MISO = PA_6（雙線）、沒有 dummy byte
├── tools/
│   └── timing.py            # 從 LA 原始資料算出 I²C／OLED／迴圈時間
└── captures/
    └── 20261031_timing.sal  # 28 個迴圈的 I²C + SPI 波形（用 Logic 2 開啟）
```

## 🛠 編譯與燒錄

在專案根目錄 `STM32_F446RE_Learning/` 下用 **PowerShell** 執行 [`build-day-from-portfolio.ps1`](../../build-day-from-portfolio.ps1)，編譯完會自動燒錄到板子：

```powershell
powershell -ExecutionPolicy Bypass -File .\build-day-from-portfolio.ps1 -Day Phase2_AttitudeSensing/04_Sensor_OLED
```

**讀取執行結果**：用根目錄的 [`serial-monitor.ps1`](../../serial-monitor.ps1)（115200）；按板子 RESET 後第一行是 `[IMU] config OK`，之後每 0.1 秒一行，數值與 OLED 相同。

```powershell
powershell -ExecutionPolicy Bypass -File .\serial-monitor.ps1
```

## 💡 學到的關鍵概念

| 重點 | 做法與結果 |
| --- | --- |
| 先預估再量測 | 從驅動的送法推算 828 byte、約 9 ms，實測 6.97 ms；差異來自預估用了單線模式的 byte 週期 |
| 一次錄兩個匯流排 | I²C 和 SPI 同時接 LA，直接看出「I²C 讀取 → OLED 更新 → 等待」的順序與各自的時間 |
| 量測工具會改變電路 | 接著 LA 時 OLED 正常、拔掉就異常；問題從 Day 6 就存在，只是一直被探針掩蓋 |
| 從收到的指令反推現象 | 用 shift1 公式推算 OLED 收到的指令，解釋不顯示、錯位、變暗各自的原因 |
| 先查最基本的接線 | 三個問題都出在接線（RES／CS 對調、兩個 GND），用 LA 的 START 波形和量導通找出來 |
| 有限字寬的顯示排版 | 每行 21 個字，固定欄寬、靠右對齊、整行重畫；minimal-printf 的 `%s` 不支援寬度，自己對齊 |

## 🔗 與其他 Day / 專案的連結

- **← Phase 2-01**：沿用 8MHz 設定與雙線 SPI 的修正；單線模式的窄脈衝就是 ④ 的根因
- **← Phase 2-03**：沿用 `lsm6ds3` driver 與 mg／mdps 換算
- **→ Phase 2-05**：在這個顯示架構上加入傾斜角度計算；[`tools/attitude_viewer.html`](../tools/README.md) 已先在電腦上驗證加速度計傾角與陀螺儀互補濾波