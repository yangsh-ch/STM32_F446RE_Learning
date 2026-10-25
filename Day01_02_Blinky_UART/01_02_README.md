# Day 1-2：開發環境建立與 Blinky + UART Debug

## 📊 總覽

| 項目 | 內容 |
| --- | --- |
| 目的 | 建立 Mbed OS 6 開發流程：編譯 → 燒錄 → 執行 → 看到輸出 |
| 實作 | `DigitalOut` 控制 LED 閃爍；`BufferedSerial` + `printf` 從 USB 虛擬 COM port 輸出訊息 |
| 結果 | ✅ LED 依設定週期閃爍；Serial Monitor（115200 baud）收到輸出 |
| 環境 | 當時使用 Keil Studio Cloud；該服務 2026/07 關閉後改用本地 CLI 工具鏈（見根目錄 README） |

## 🧩 兩個版本

| 版本 | 寫法 |
| --- | --- |
| 實作一：Blinky | `myled = 1/0`、`thread_sleep_for(500)` |
| 實作二：Toggle + UART | `myled = !myled`、`ThisThread::sleep_for(1000ms)`、`static BufferedSerial`（Mbed OS 6 建議寫法） |

## 💡 學到的重點

| 重點 | 說明 |
| --- | --- |
| 先打通工具鏈 | 確認編譯、燒錄、執行、輸出整條流程可用，後續實驗都建立在這上面 |
| `printf` 是最快的除錯管道 | 走 USB 虛擬 COM port，不用額外接線 |
| 使用 Mbed OS 6 API | `BufferedSerial` 取代舊版 `Serial`，`ThisThread::sleep_for` 取代 `thread_sleep_for` |
