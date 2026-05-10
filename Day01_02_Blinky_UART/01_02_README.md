# 學習目標

- 建立 Mbed OS 開發環境（Keil Studio Cloud）
- 使用 `DigitalOut` 控制 LED 閃爍
- 使用 `printf` 與 `BufferedSerial` 輸出 UART debug 訊息
- 驗證開發板與電腦的 USB Serial 通訊

---

# 實作版本

| 版本 | 重點 |
| --- | --- |
| 實作一：Blinky | `myled = 1/0`，`thread_sleep_for(500)` |
| 實作二：BufferedSerial + Toggle | `myled = !myled`，`ThisThread::sleep_for(1000ms)`，`static BufferedSerial` |

> 兩者等效，實作二更接近 Mbed OS 6 的正式寫法。
> 

---

# 驗證結果

- LED 以對應週期正常閃爍
- Serial Monitor（115200 baud）收到對應輸出訊息
- 開發環境確認可正常編譯、燒錄、執行

---

# 學到的關鍵概念
| 概念 | 說明 |
| --- | --- |
| `DigitalOut` | GPIO 輸出控制 |
| `BufferedSerial` | 非同步 UART 緩衝傳輸，取代舊版 `Serial` |
| `thread_sleep_for` vs `ThisThread::sleep_for` | 兩者等效，後者為 Mbed OS 6 推薦語法 |
| `printf` debug | 最快速的韌體除錯方式，輸出走 USB Virtual COM Port |