# Phase 2-02：I²C 與 LSM6DS3 — Address Scan、WHO_AM_I 與暫存器讀寫

## 📊 測試總覽

| 項目 | 內容 |
| --- | --- |
| 目的 | 用 I²C 的 ACK／NACK 縮小故障範圍：先確認位址與晶片版本，再寫暫存器讀寫的 driver 骨架 |
| 硬體 | NUCLEO-F446RE 的 I2C1（PB8 = SCL、PB9 = SDA）接 GY-LSM6DS3 模組；CS 接 3.3V（I²C 模式），SA0 空接（實測為高） |
| 測試方式 | 單次燒錄，開機後依序執行：① 掃描 0x08～0x77 → ③ 讀 `WHO_AM_I`（0x0F）→ ⑤ 寫入 `CTRL1_XL`（0x10）後讀回；④ 改成 400kHz 再跑一次 |
| 輸出 | `printf` 走 ST-LINK 虛擬 COM port（9600 baud） |
| 量測 | Saleae Logic 2，24 MS/s；I2C Analyzer（SCL = CH0、SDA = CH1） |

## 🔍 結果

### 功能

| 項目 | 預期 | 100kHz | 400kHz |
| --- | --- | --- | --- |
| 回 ACK 的位址 | 只有 0x6B | ✅ 只有 0x6B | ✅ |
| NACK 位址的第 9 個 clock | SDA 維持高 | ✅ 0x77：NAK | ✅ |
| 位址 byte（線上） | 0xD6 | ✅ Logic 2 以 7-bit 顯示為 `[0x6B] Write` | ✅ |
| `WHO_AM_I` | 0x69（TR-C 為 0x6A） | ✅ **0x69 → LSM6DS3** | ✅ |
| 寫入後讀回 `CTRL1_XL` | 寫入值 = 讀回值 | ✅ 寫 0x10、讀回 0x10；before = 0x00 | ✅ |

終端機輸出（修正後）：

```
[I2C] scan 0x08-0x77
  found 0x6B (wire W=0xD6 R=0xD7)
[I2C] WHO_AM_I = 0x69 (write=0, read=0)
[I2C] CTRL1_XL before=0x00 write=0x10 readback=0x10 -> MATCH
```

### 時序對照規格（UM10204 Table 11）

| 程式設定 | 實際 SCL | tLOW | 規格下限 | tHIGH | 規格下限 |
| --- | --- | --- | --- | --- | --- |
| 100kHz | 100 kHz（週期 10 µs） | 5.29 µs ✅ | 4.7 µs | 4.71 µs ✅ | 4.0 µs |
| 400kHz | 352.9 kHz（週期 2.833 µs） | 2.04 µs ✅ | 1.3 µs | 0.79 µs ✅ | 0.6 µs |

- 量法：讀 Logic 2 在 SCL 上顯示的單一週期頻率與高電位寬度，tLOW = 週期 − tHIGH；兩個速度都用 24 MS/s 量測，±1 個取樣點 = ±41.7 ns
- 400kHz 實際約 353 kHz（低約 12%），原因**未確定**（先前用 12 MS/s 量到 363.6 kHz，差異在 12 MS/s 的 1 個取樣點內）
- 兩個速度的 tHIGH 都比 tLOW 短（100kHz 依設計兩者應相等），可能與上升時間有關，需要示波器確認

## 🐛 問題與根因

| # | 現象 | 根因 |
| --- | --- | --- |
| ① | ③ 讀 `WHO_AM_I` 得到 0x00（`write=1`），但 ⑤ 用同樣流程讀寫 `CTRL1_XL` 正常 | 掃描最後一筆（0x77）收到 NACK，留下 **AF 旗標**。mbed 單 byte 版的 `i2c_start()` 會清 AF、`i2c_stop()` 不會；完整版 `write()` 呼叫的 `HAL_I2C_Master_Seq_Transmit_IT()` 送 START 前也不清 AF，錯誤中斷立刻觸發 → 線上只有 START 後 17 µs 就 STOP，`0x0F` 沒送出去 |
| ② | Logic 2 擷取出現 ReadTimeout | PowerShell 同時監聽 COM5 時才會發生，停掉監聽後就正常（24／12 MS/s 皆同，重複多次）。分析儀和 Nucleo 透過同一條 USB 延長線接到電腦；機制**未確定**，改接電腦本體不同 USB 孔是否就不發生**還沒驗證** |

- **解法 ①（已驗證）**：掃描結束後手動清除 AF：`I2C1->SR1 &= ~I2C_SR1_AF;`。修改後線上序列變成 `START → [0x6B]W ACK → 0x0F ACK → Sr → [0x6B]R ACK → 0x69 NAK → STOP`
- **解法 ②（已驗證）**：用邏輯分析儀擷取前先停掉 COM5 監聽（Ctrl+C）；看終端機輸出和錄波形分開做

## ✅ 結論

- 接線、位址（0x6B）、晶片版本（LSM6DS3）、讀取序列都已實測確認，Phase 2-03 可以直接讀加速度／陀螺儀資料
- 用 ACK／NACK 縮小範圍：掃描有 ACK → 接線與位址沒問題；③ 失敗但 ⑤ 正常 → 問題在軟體狀態（AF 旗標），不在硬體
- 100kHz、400kHz 都能正常通訊，時序都在 UM10204 規格內
- **限制**：pull-up 實測約 10 kΩ，400kHz 時線上電容要低於約 35 pF 才符合 tr 規格；邏輯分析儀量不到 tr，需要示波器確認

## 📁 檔案結構

```
Phase2_AttitudeSensing/02_I2C_LSM6DS3/
├── 02_main.cpp            # Address Scan → WHO_AM_I → 寫入後讀回
└── Core/
    ├── Inc/
    │   └── lsm6ds3.h      # 位址常數與 driver 介面
    └── Src/
        └── lsm6ds3.cpp    # 暫存器讀寫（共用 main 建立的 I2C 物件）
```

## 🛠 編譯與燒錄

在專案根目錄 `STM32_F446RE_Learning/` 下用 **PowerShell** 執行 [`build-day-from-portfolio.ps1`](../../build-day-from-portfolio.ps1)，編譯完會自動燒錄到板子：

```powershell
powershell -ExecutionPolicy Bypass -File .\build-day-from-portfolio.ps1 -Day Phase2_AttitudeSensing/02_I2C_LSM6DS3
```

**讀取執行結果**：程式只在開機時跑一次，按 RESET 後會從 ST-LINK 虛擬 COM port 印出 4 行結果（掃描到的位址、`WHO_AM_I`、寫入後讀回，見上方「結果」）。用根目錄的 [`serial-monitor.ps1`](../../serial-monitor.ps1)（會自動找 ST-LINK 的 COM port），這一版是 **9600 baud**，要加 `-Baud 9600`；執行後按板子 RESET，要結束按 Ctrl+C。用邏輯分析儀擷取前要先停掉監聽（見解法 ②）。

```powershell
powershell -ExecutionPolicy Bypass -File .\serial-monitor.ps1 -Baud 9600
```

## 💡 學到的關鍵概念

| 重點 | 做法與結果 |
| --- | --- |
| 用 ACK／NACK 縮小故障範圍 | 掃描有 ACK → 接線、位址沒問題；`WHO_AM_I` 失敗但暫存器讀寫正常 → 問題在軟體狀態，不在硬體 |
| 從波形追到驅動原始碼 | `WHO_AM_I` 讀到 0x00：波形顯示 START 後 17 µs 就 STOP，追進 mbed／HAL 原始碼，找到掃描 NACK 留下的 AF 旗標，清除後修正 |
| 先實測再寫 driver | Address Scan 確認位址 0x6B（掃 0x08～0x77，避開 UM10204 保留位址），`WHO_AM_I` 確認晶片是 LSM6DS3 |
| 寫入測試要能分辨成敗 | 避開預設值 0x00（失敗也讀回 0x00），也避開 `I2C_disable` 這類會切斷通訊的位元 |
| 時序對照規格 | 100kHz／400kHz 的 tLOW、tHIGH 都在 UM10204 規格內；由 pull-up 10 kΩ 推算 400kHz 線上電容需 < 35 pF |

## 🔗 與其他 Day / 專案的連結

- **← Phase 2-01**：沿用「從程式推算預期 → LA 實測 → 對照」的驗證流程，以及「第 1 → 第 8 上升緣」等量測經驗
- **← Day4**：UART 除錯輸出的概念；這次 `printf` 改走 ST-LINK 虛擬 COM port
- **→ [Phase 2-03](../03_Sensor_Processing/03_README.md)**：在已確認的位址與讀寫函式上，讀取加速度／陀螺儀原始值並換算成 mg、mdps
