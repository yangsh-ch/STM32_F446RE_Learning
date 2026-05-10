# 學習目標

- 使用 `DigitalIn` 讀取按鈕狀態（Polling）
- 依按鈕輸入控制 LED toggle
- 理解機械按鈕彈跳（Bounce）問題，並實作軟體 Debounce
- 透過 UART 輸出 LED 狀態變化紀錄

---

# Debounce 設計說明

採用三段式軟體 Debounce，防止一次按壓觸發多次 toggle：

```
按下偵測（button == 0）
    ↓
執行 toggle + printf
    ↓
sleep 50ms（按下防彈跳）
    ↓
等待按鈕放開（while(button == 0)）
    ↓
sleep 50ms（放開防彈跳）
    ↓
回到 while(1) 繼續 Polling
```

| 步驟 | 目的 |
| --- | --- |
| 按下後 sleep 50ms | 忽略按下瞬間的機械彈跳雜訊 |
| `while(button == 0)` | 等待放開，防止持續觸發 |
| 放開後 sleep 50ms | 忽略放開瞬間的機械彈跳雜訊 |

---

# 驗證結果

- 每次按下按鈕，LED 正確 toggle 一次（不會連跳）
- UART 輸出對應狀態：`LED is now ON` / `LED is now OFF`
- BUTTON1 為 **Active Low**：按下時讀到 `0`，放開時讀到 `1`

---

# 學到的關鍵概念

| 概念 | 說明 |
| --- | --- |
| `DigitalIn` | GPIO 輸入讀取 |
| Active Low | BUTTON1 按下為低電位（0），與直覺相反 |
| 軟體 Debounce | 三段式延遲防止機械彈跳誤觸 |
| Polling 的限制 | `while(1)` 持續佔用 CPU，無法做其他事 |

---

# 與 Day 5 的連結

此程式的 Polling 架構在 Day 5 被中斷（Interrupt）取代：

- **Polling**：CPU 一直主動檢查 → 浪費資源
- **Interrupt**：事件發生才通知 CPU → 效率更高，延遲更低