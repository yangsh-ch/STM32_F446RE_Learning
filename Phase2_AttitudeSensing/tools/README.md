# Phase 2 tools：即時 3D 姿態檢視

轉動實體的 LSM6DS3，網頁上的 3D 模型即時跟著動。用來在寫 2-05 傾斜角度之前，先在電腦上確認軸向、正負號與演算法。

## 組成

| 檔案 | 用途 |
| --- | --- |
| [`imu_stream/imu_stream_main.cpp`](imu_stream/imu_stream_main.cpp) | 韌體：每一筆 104Hz 的資料都從 UART 送出（不更新 OLED），格式 `T[us] … A[mg] … G[mdps] …`，多了 MCU 時間戳記；`lsm6ds3` driver 沿用 2-04 |
| [`attitude_viewer.html`](attitude_viewer.html) | 網頁：用 Web Serial 讀 COM port，解析每一行並畫出 3D 模型 |

- 實測 3 秒收到 308 行，約 103 Hz；每行約 65 字，115200 baud 足夠
- 網頁也看得懂 2-03／2-04 的輸出（沒有時間戳記時改用收到的時間算 dt），但那兩支每 100 ms 才印一行，B 模式不準

## 兩種模式

| 模式 | 算法 | 能看到 | 限制 |
| --- | --- | --- | --- |
| A：加速度計 | roll = atan2(ay, az)，pitch = atan2(−ax, √(ay² + az²)) | Roll、Pitch | 水平轉動（Yaw）看不到；晃動時加速度不只有重力，角度會跳 |
| B：陀螺儀 + 互補濾波 | 四元數積分陀螺儀，再用加速度計把 Roll／Pitch 慢慢拉回（Mahony，Kp = 1）；\|a\| 偏離 1 g 超過 20% 時只用陀螺儀 | Roll、Pitch、Yaw | Yaw 沒有參考值修正，會慢慢漂移，用「Yaw 歸零」重設 |

- 連線後先取 200 筆（約 2 秒）陀螺儀平均當作零點偏移，這段時間感測器要靜止；之後可按「重新校正」

## 使用方式

1. 燒錄韌體（OLED 會停在上一個畫面，正常）：
   ```powershell
   powershell -ExecutionPolicy Bypass -File .\build-day-from-portfolio.ps1 -Day Phase2_AttitudeSensing/tools/imu_stream
   ```
2. Web Serial 不能在 `file://` 使用，要從本機伺服器開網頁：
   ```powershell
   python -m http.server 8765 --bind 127.0.0.1 --directory Phase2_AttitudeSensing/tools
   ```
3. 用 Chrome／Edge 開 `http://localhost:8765/attitude_viewer.html`，感測器放平靜止，按「連線」選 ST-LINK 的 COM port
4. 要回到 2-04，重新燒錄 `04_Sensor_OLED`；用網頁時要先關掉 `serial-monitor.ps1`（同一個 COM port 只能一個程式開）

## 驗證

| 測試 | 結果 |
| --- | --- |
| A：實機前後左右傾斜 | ✅ 模型跟著傾斜（用 2-04 韌體，更新頻率 9.3 Hz，與迴圈週期 108 ms 一致） |
| B：實機水平轉動、前後左右傾斜、轉動後放下 | ✅ 102.5 Hz；前後左右傾斜 Roll 到 ±58°、Pitch 到 +38°；水平轉動，截圖中 Yaw 最大 +148.2°、最小 −48.5°；轉動後放下，Roll／Pitch 穩定在 12.2°／−16.6° |

- 另以模擬資料驗證算法（未拍攝）：繞 X 轉 +90° → Roll 90.3°；繞 Z 以 90 dps 轉 1 秒、再靜止 5 秒 → Yaw 90.0°，靜止時沒有漂移
