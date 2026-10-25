# Day 3：GPIO 輸入輸出與軟體 Debounce

## 📊 總覽

| 項目 | 內容 |
| --- | --- |
| 目的 | 用 `DigitalIn` Polling 讀按鈕、控制 LED toggle，並處理機械按鈕彈跳 |
| 結果 | ✅ 每按一次 LED 只 toggle 一次（不連跳），UART 輸出 `LED is now ON` / `LED is now OFF` |
| 發現 | BUTTON1 是 **Active Low**：按下讀到 0、放開讀到 1 |

## 🔧 三段式軟體 Debounce

| 步驟 | 做法 | 目的 |
| --- | --- | --- |
| ① | 偵測到按下（`button == 0`）→ toggle 並輸出 → `sleep 50ms` | 忽略按下瞬間的彈跳 |
| ② | `while (button == 0)` | 等放開，避免按住時連續觸發 |
| ③ | 放開後 `sleep 50ms` | 忽略放開瞬間的彈跳 |

## 💡 學到的重點

| 重點 | 說明 |
| --- | --- |
| Active Low | 按下為 0，判斷條件要反過來寫 |
| 軟體 Debounce | 用延遲濾掉機械彈跳，一次按壓只觸發一次 |
| Polling 的代價 | `while(1)` 一直佔用 CPU，等按鈕時不能做別的事 |

## 🔗 與其他 Day 的連結

- **→ Day 5**：Polling 改成中斷（Interrupt），事件發生才通知 CPU
