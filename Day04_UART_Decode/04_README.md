# 學習目標

- 使用邏輯分析儀捕捉 UART TX 波形
- 實作模組化 UART 封裝（`uart_debug.h` / `uart_debug.cpp`）
- 理解 UART Frame 結構：Start bit / Data bits / Stop bit
- 驗證 baud rate 與 bit 時間寬度

---

# 檔案結構

```cpp
Day04_UART_Decode/
├── main.cpp
├── Core/
│   ├── Inc/
│   │   └── uart_debug.h
│   └── Src/
│       └── uart_debug.cpp
```

---

# 模組化設計說明

| 直接用 `BufferedSerial` | 封裝成 `uart_send_log()` |
| --- | --- |
| 每次都要 `.write()`  • 計算長度 | 一行呼叫，介面統一 |
| 散落各處，難以維護 | 集中管理，日後可擴充 timestamp、log level |
| 換腳位要改很多地方 | 只需改 `uart_debug.cpp` 一處 |

> 此模組化風格在 Day 5 的 `exti_control.h` 繼續沿用。
> 

---

# 硬體設定

| 項目 | 設定 |
| --- | --- |
| TX 腳位 | PA_9 |
| RX | NC（不使用） |
| Baud Rate | 9600 bps |
| 邏輯分析儀 CH1 | 接 PA_9，捕捉 TX 波形 |

---

# UART Frame 解析

| 參數 | 值 | 說明 |
| --- | --- | --- |
| Baud Rate | 9600 bps | 每秒傳 9600 個 bit |
| 每 bit 時間 | ≈ 104.2 µs | 邏輯分析儀可直接量測驗證 |
| 資料順序 | LSB first | 最低位元先傳 |
| Start bit | 低電位 | 通知接收端開始 |
| Stop bit | 高電位 | 通知接收端結束 |

---

# 學到的關鍵概念

| 概念 | 說明 |
| --- | --- |
| `BufferedSerial(PA_9, NC, 9600)` | 指定腳位、不用 RX、設定 baud rate |
| `serial_port.write(msg, len)` | 非同步寫入，不 blocking |
| UART 非同步通訊 | 雙方約定好 baud rate，不需要額外時脈線 |
| LSB first | UART 資料從最低位元開始傳 |
| 模組化封裝 | `.h` 定義介面，`.cpp` 實作細節，`main.cpp` 只呼叫 API |

---

# 與其他 Day 的連結

- **← Day 3**：debug 訊息從 `printf` 改為獨立 UART 模組，封裝更乾淨
- **→ Day 5**：`uart_debug_init()` 在 Day 5 的 `main.cpp` 被 comment 掉，原因是 Day 5 專注測試中斷時序，UART 輸出會干擾量測