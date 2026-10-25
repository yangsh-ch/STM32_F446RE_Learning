# Day 4：UART 模組化與邏輯分析儀解碼

## 📊 總覽

| 項目 | 內容 |
| --- | --- |
| 目的 | 把 UART 輸出封裝成獨立模組，並用邏輯分析儀捕捉 TX 波形、對照 UART frame 結構 |
| 硬體 | TX = PA_9（RX 不用），9600 baud；邏輯分析儀 CH1 接 PA_9 |
| 實作 | `uart_debug.h/.cpp` 封裝 `BufferedSerial`，`main.cpp` 只呼叫 `uart_send_log()` |

## 🔍 UART Frame（9600 baud）

| 項目 | 值 |
| --- | --- |
| 每 bit 時間 | 1 / 9600 ≈ 104.2 µs |
| Start bit | 低電位，通知接收端開始 |
| 資料順序 | LSB first（最低位元先傳） |
| Stop bit | 高電位，通知接收端結束 |

## 🧩 為什麼要模組化

| 直接用 `BufferedSerial` | 封裝成 `uart_send_log()` |
| --- | --- |
| 每次都要呼叫 `.write()` 並自己算長度 | 一行呼叫，介面統一 |
| 散落各處，難以維護 | 集中管理，之後可加 timestamp、log level |
| 換腳位要改很多地方 | 只改 `uart_debug.cpp` 一處 |

這種「`.h` 介面 + `.cpp` 實作」的拆法，Day 5、Day 6 都沿用。

## 📁 檔案結構

```
Day04_UART_Decode/
├── main.cpp
└── Core/
    ├── Inc/
    │   └── uart_debug.h
    └── Src/
        └── uart_debug.cpp
```

## 💡 學到的重點

| 重點 | 說明 |
| --- | --- |
| UART 是非同步通訊 | 雙方約定 baud rate，不需要時脈線；bit 寬度由 baud rate 決定 |
| 用波形對照協定 | 在邏輯分析儀上辨認 Start bit、LSB first 的資料位元與 Stop bit |
| 介面與實作分離 | `.h` 定義介面、`.cpp` 實作細節，`main.cpp` 只呼叫 API |

## 🔗 與其他 Day 的連結

- **← Day 3**：debug 訊息從直接 `printf` 改成獨立的 UART 模組
- **→ Day 5**：`uart_debug_init()` 在 Day 5 被註解掉，因為 UART 輸出會干擾中斷時序量測
