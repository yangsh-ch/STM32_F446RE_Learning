# Phase 2-01：SPI 頻率測試 — 1/4/8/16MHz 逐 byte 驗證與時序量測

## 📊 測試總覽

| 項目 | 內容 |
| --- | --- |
| 目的 | 在杜邦線飛線的接法下，實測提高 SPI 頻率後資料是否仍正確，並對照 SSD1306 datasheet 驗證時序 |
| 硬體 | 沿用 Day6 的 OLED 接線（PA_7 MOSI / PA_5 SCLK / PB_10 CS / PB_1 DC / PB_0 RES），另將 UART TX（PA_9）接到邏輯分析儀 |
| 測試方式 | 單次燒錄，依序切換 1→4→8→16MHz；每段先清屏、畫四行字，再從 PA_9 送出 `[SPI] <Hz> Hz, cycle <N>` 標籤，接著閒置 5 秒 |
| 量測 | Saleae Logic 2，24 MS/s；SPI Analyzer 的 MISO 設為 DC（CH3），解碼表直接標出每個 byte 是指令還是資料 |
| 驗證方式 | [`tools/spi_compare.py`](tools/spi_compare.py) 從原始碼推算每個 byte 應該送什麼，與邏輯分析儀匯出的 CSV 逐 byte 比對 |

## 🔍 結果

### 逐 byte 比對（[`captures/20261012_*`](captures/20261012_report.md)）

| 頻率段 | 預期 byte | 實際 byte | ok | shift1 | 其他不符 | 缺少 |
| --- | --- | --- | --- | --- | --- | --- |
| 1MHz（含 init） | 1563 | 1561 | 1164 | 397 | **0** | 2 |
| 4MHz | 1546 | 1545 | 1113 | 432 | **0** | 1 |
| 8MHz | 1546 | 1545 | 1097 | 448 | **0** | 1 |
| 16MHz | 1558 | 1557 | 1107 | 450 | **0** | 1 |
| 1MHz（cycle 1） | 1546 | 1545 | 1137 | 408 | **0** | 1 |

7,759 個 byte 中「其他不符」為 0：每一筆不符都只有兩種型態（見下方問題與根因）。

### 時序對照 datasheet

| 程式設定 | 實測 SCLK | T | t_cycle ≥ 100ns | tCSS | tCSS ≥ 20ns |
| --- | --- | --- | --- | --- | --- |
| 1MHz | 705.9kHz | 1417ns | ✅ | 5.426µs | ✅ |
| 4MHz | 2.80MHz | 357.1ns | ✅ | 5.005µs | ✅ |
| 8MHz | 5.60MHz | 178.6ns | ✅ | 4.924µs | ✅ |
| 16MHz | 11.2MHz | **89.3ns** | ❌ 低於下限約 11% | 4.877µs | ✅ |

- 實際頻率不等於設定值：SPI1 時脈來源是 90MHz，mbed 只能做 2 的次方分頻，會挑「不超過設定值的最高頻率」（703k / 2.81M / 5.63M / 11.25MHz）
- T 的量法：4/8/16MHz 量「第 1 個 → 第 8 個上升緣」再除以 7；高頻時一個週期只有 2～4 個取樣點，量單一週期會失準

## 🐛 問題與根因

| # | 現象 | 根因 | OLED 有受影響嗎 |
| --- | --- | --- | --- |
| ① | 每段約 25～29% 的 byte 解碼值 = `0x80 \| (預期 >> 1)`（例：`0x8D`→`0xC6`） | byte 開頭有一根約一個取樣點寬的窄 SCLK 脈衝，被分析儀多算成第 1 個 bit（9 根脈衝、8 個 ▲）；用 8 根正常 clock 手動解碼為正確值。比例會隨 USB 連接狀態變動（見下方觀察）；窄脈衝的來源**未確定** | 否：開機畫面有亮（`0x8D` 有收到）、清屏後沒有虛線 |
| ② | 每次 `spi.frequency()` 後的第一個 byte 沒被解碼（init 的 `0xAE`、每段清屏的 `0x21`） | 改頻率後 SCLK 停在高電位，CS 拉低時不符合 Mode 0 的閒置低電位，分析儀判定 idle state mismatch 而放棄解碼；手動解碼為 `0x21`，MCU 有送 | 否：畫面沒有錯位 |

- **解法 ②（已驗證）**：`oled_set_spi_frequency()` 改完頻率後，先在 CS 為高的狀態下送一個 dummy byte，讓 SCLK 回到閒置低電位；`oled_init()` 也改走同一個函式。CS 要先明確拉高，因為 `DigitalOut oled_cs(PB_10)` 開機預設是 0

### 解法 ② 驗證（[`captures/20261015_fix_*`](captures/20261015_fix_report.md)）

| 頻率段 | 實際 byte（預期） | 缺少 | 多出 | 其他不符 |
| --- | --- | --- | --- | --- |
| 1MHz（含 init） | 1561 → **1563**（1563） | 2 → **0** | 0 → 0 | 0 → 0 |
| 4MHz | 1545 → **1546**（1546） | 1 → **0** | 0 → 0 | 0 → 0 |
| 8MHz | 1545 → **1546**（1546） | 1 → **0** | 0 → 0 | 0 → 0 |
| 16MHz | 1557 → **1558**（1558） | 1 → **0** | 0 → 0 | 0 → 0 |
| 1MHz（cycle 1） | 1545 → **1546**（1546） | 1 → **0** | 0 → 0 | 0 → 0 |

- 每段的實際 byte 數都等於預期值；dummy byte 送出時 CS 為高，分析儀不會解碼它（多出 = 0），OLED 畫面也正常

### 重複驗證與 shift1 比例的觀察

| 擷取 | 程式版本 | 與上一次之間 | 缺少（每段） | shift1 比例 |
| --- | --- | --- | --- | --- |
| 10/12 | 修改前 | — | 1（開機段 2） | 25～29% |
| fix | 修改後 | 拔插 USB | **0** | 64～68% |
| recover #1 | 修改前 | 沒拔插（只拖檔燒錄） | 1（開機段 2） | 62～66%（原始檔未保留） |
| recover_test | 修改前 | 拔插 USB | 1（開機段 2） | 49～51% |
| fix2 | 修改後 | 沒拔插（只燒錄） | **0** | 47～53% |
| fix3 | 修改後 | 沒拔插（什麼都沒動） | **0** | 48～52% |
| fix4 | 修改後 | 拔插 USB | **0** | 16～17% |
| fix5 | 修改後 | 拔插 USB | **0** | 17～19% |

- **缺少**：修改後 5 次全部是 0，修改前 3 次每次都是每段 1（開機段 2），來回切換版本都能重現 → 解法 ② 的效果確定
- **shift1 比例跟程式版本、SPI 頻率無關**：不拔插 USB 時，不論燒的是哪個版本，比例都維持在同一水準（約 65% 或約 50%）；同一次擷取內，四個頻率的比例也都很接近
- **拔插 USB 會讓比例變動**：4 次拔插中，3 次明顯改變（27→65、65→50、50→16），1 次只差約 2 個百分點（16→18）
- 比例取決於當下的 USB 連接狀態，但窄脈衝本身是怎麼產生的，仍未確定

## ✅ 結論

- 四個頻率下，程式送出的 byte **全部正確到達 OLED**；邏輯分析儀看到的錯誤都來自分析儀本身的解碼
- 原本預期的「漸進式劣化」沒有出現；不過實際測試的最高頻率只超出上限約 12%，不是規劃時以為的 60%
- 16MHz 設定（實際 11.25MHz）已低於 datasheet 的 t_cycle 下限：這組硬體能動，但不在規格保證範圍內
- 規格內最快的是 8MHz 設定（實際 5.63MHz）；但每個 byte 有約 9～10µs 的固定軟體開銷，byte 週期只從 20.0µs 縮短到 11.0µs
- 分析儀「缺少 byte」的問題（根因 ②）已用 dummy byte 解決，修改後 5 段的「缺少」都是 0
- **限制**：24 MS/s 的邏輯分析儀只看得到 0/1，看不到邊緣與振鈴等類比波形；要評估訊號完整性的餘裕需要示波器

## 🔬 比對工具 `tools/spi_compare.py`

1. 從 `Core/Src/oled_ssd1306.cpp` 讀出字型表與 `oled_init()` 指令，照 `01_main.cpp` 的流程推算每段應送出的 byte（含指令/資料）
2. 讀 Logic 2 匯出的 SPI CSV（MOSI + MISO=DC）與 UART CSV（頻率段標籤）
3. 用 UART 標籤切成各頻率段，逐 byte 對齊並分類：ok / shift1 / shift0 / 其他不符 / 缺少 / 多出
4. 輸出 Markdown 報告

使用前先跑 `--selftest`：用預期值造假資料並埋入已知錯誤，確認工具每一種都抓得到。

```powershell
python Phase2_AttitudeSensing/01_SPI_Optimization/tools/spi_compare.py --selftest
python Phase2_AttitudeSensing/01_SPI_Optimization/tools/spi_compare.py <spi.csv> <uart.csv> --out <report.md>
```

## 📁 檔案結構

```
Phase2_AttitudeSensing/01_SPI_Optimization/
├── 01_main.cpp
├── Core/
│   ├── Inc/
│   │   └── oled_ssd1306.h
│   └── Src/
│       └── oled_ssd1306.cpp
├── tools/
│   └── spi_compare.py
└── captures/
    ├── 20261012_spi.csv        # Logic 2 匯出：SPI 解碼結果（解法 ② 修改前）
    ├── 20261012_uart.csv       # Logic 2 匯出：UART 標籤
    ├── 20261012_report.md      # spi_compare.py 的比對報告
    ├── 20261015_fix_*          # 解法 ② 修改後的擷取（spi.csv / uart.csv / report.md）
    ├── 20261015_fix2～fix5_*   # 修改後的重複擷取（fix4、fix5 前拔插 USB）
    └── 20261015_recover_test_* # 燒回修改前版本的擷取
```

## 🛠 編譯與燒錄

在專案根目錄 `STM32_F446RE_Learning/` 下用 **PowerShell** 執行 [`build-day-from-portfolio.ps1`](../../build-day-from-portfolio.ps1)，編譯完會自動燒錄到板子：

```powershell
powershell -ExecutionPolicy Bypass -File .\build-day-from-portfolio.ps1 -Day Phase2_AttitudeSensing/01_SPI_Optimization
```

## 💡 學到的關鍵概念

| 概念 | 說明 |
| --- | --- |
| SSD1306 SPI 時脈上限 | Datasheet Table 13-4，t_cycle 最小 100ns → 最高 10MHz |
| 設定頻率 ≠ 實際頻率 | 分頻器只能做 2 的次方，實際頻率要量 SCLK 週期才知道 |
| 取樣率決定能量多準 | 每個週期至少要 4 個取樣點才能穩定解碼；高頻改量多個週期再平均 |
| 分析儀解碼錯 ≠ 裝置收錯 | 要用裝置本身的反應（OLED 畫面）交叉確認，才能判斷是誰出錯 |
| 先有預期值，才能驗證 | 從程式推算每個 byte 應該是什麼，再用工具逐 byte 比對，人只看不符的部分 |
| 軟體開銷主導傳輸速度 | `wait_us()` 與函式呼叫的固定開銷，讓 SCLK 加快 16 倍時 byte 週期只快約 1.8 倍 |

## 🔗 與其他 Day / 專案的連結

- **← Day6**：沿用 Day6 的 OLED 接線與 driver，只新增 `oled_set_spi_frequency()`，讓 `01_main.cpp` 能在執行中切換 SPI 頻率
- **← Day4**：延續 UART 除錯訊號固定走 PA_9 的慣例，方便外接邏輯分析儀
- **→ 後續感測器整合**：「從程式推算預期值 → 匯出解碼結果 → 工具逐 byte 比對」的流程，之後接 LSM6DS3 等感測器時可以沿用
