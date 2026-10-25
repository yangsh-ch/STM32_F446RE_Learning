# STM32 F446RE Firmware Learning Journey

> Hands-on embedded firmware development using STM32F446RE (NUCLEO-F446RE) with Mbed OS.
> Focused on low-level peripheral control, interrupt-driven design, and digital power control — targeting server PSU firmware applications.

## 🎯 Project Goal

Bridge from communication-layer firmware (UART/PCIe/I2C) to **digital power control firmware** relevant to server infrastructure (PSU, data center).

## 🛠 Hardware Setup

| Component | Model | Role |
| --- | --- | --- |
| MCU Board | NUCLEO-F446RE (Cortex-M4 @ 180MHz) | Main controller |
| IMU Module | GY-LSM6DS3 (6-axis accel + gyro, I²C) | Attitude sensing (Phase 2) |
| OLED Display | SSD1306 128×64 (SPI) | Text / data display (Day 6, Phase 2) |
| Logic Analyzer | 8-channel, 24 MS/s | Signal verification & timing measurement |

## 📁 Repository Structure

```
STM32_F446RE_Learning/
├── Day01_02_Blinky_UART/    # GPIO output + Serial debug
├── Day03_GPIO/              # Digital input/output & debounce
├── Day04_UART_Decode/       # UART capture with logic analyzer
├── Day05_Interrupt/         # ISR design, latency measurement (170ms → <50µs)
├── Day06_OLED_SPI/          # SPI protocol + SSD1306 OLED text display driver
├── Phase2_AttitudeSensing/  # [WIP] IMU/attitude track — SPI signal integrity, I2C+LSM6DS3, sensor fusion, tilt meter
├── Phase3_PowerCtrl/        # [Planned] ADC + PWM + PID + PMBus digital power control
└── docs/waveforms/          # Logic analyzer screenshots
```

## 📈 Progress Log

### ✅ Phase 1 — Core Peripherals (Complete)

| Day | Topic | Key Result |
| --- | --- | --- |
| Day 1-2 | Blinky + UART | GPIO output + `printf` debug via USB Serial |
| Day 3 | GPIO I/O | Button polling + LED toggle with software debounce |
| Day 4 | UART + Logic Analyzer | Captured TX waveform, decoded Start/Stop bits (9600 baud) |
| Day 5 | Interrupt (EXTI) | **Reduced latency from 170ms (mechanical bounce) to <50µs (digital loopback)** — verified via logic analyzer |
| Day 6 | SPI + OLED (SSD1306) | Wrote SSD1306 driver from datasheet — init sequence, framebuffer paging, 5×7 bitmap font rendering over SPI |

### 🔄 Phase 2 — Attitude Sensing System (In Progress)

| Module | Topic | Status |
| --- | --- | --- |
| 01 | SPI signal integrity (1/4/8/16MHz) | ✅ Done — see [Phase2_AttitudeSensing/01_SPI_Optimization](Phase2_AttitudeSensing/01_SPI_Optimization/01_README.md) |
| 02 | I2C + LSM6DS3 (6-axis IMU) | ✅ Done — see [Phase2_AttitudeSensing/02_I2C_LSM6DS3](Phase2_AttitudeSensing/02_I2C_LSM6DS3/02_README.md) |
| 03 | Sensor data processing (raw → mg/mdps) | ✅ Done — see [Phase2_AttitudeSensing/03_Sensor_Processing](Phase2_AttitudeSensing/03_Sensor_Processing/03_README.md) |
| 04 | Sensor + OLED integration | ⏳ Planned |
| 05 | Tilt/attitude meter | ⏳ Planned |
| 06 | TTP223 touch UI (page switching) | ⏳ Planned |
| 07 | SPI DMA optimization | ⏳ Planned |
| 08 | FreeRTOS (multi-task) | ⏳ Planned |

### ⏳ Phase 3 — Digital Power Control (Planned)

| Module | Topic | Status |
| --- | --- | --- |
| 01 | ADC voltage sampling + OVP protection | ⏳ Planned |
| 02 | TIM + PWM output (100kHz) | ⏳ Planned |
| 03 | PID closed-loop voltage control | ⏳ Planned |
| 04 | PMBus (I2C) — READ_VOUT, STATUS_BYTE | ⏳ Planned |

## 🔬 Key Technical Findings

| Module | Finding | How it was verified |
| --- | --- | --- |
| Day 5 | Button latency of 140~200ms came from mechanical bounce + Mbed OS debounce, not the ISR; a digital loopback responds in **10~50µs** | Logic analyzer |
| Phase 2-01 | All 7,759 SPI bytes reached the OLED correctly; the 16MHz setting actually runs at 11.25MHz, below the SSD1306 t_cycle minimum | Byte-by-byte compare tool (`spi_compare.py`) |
| Phase 2-02 | `WHO_AM_I` read 0x00 because the NACKed last scan probe left the I2C AF flag set; clearing it fixed the next transfer | Before/after captures + mbed/HAL source |
| Phase 2-03 | Accel/gyro conversion validated against gravity: sensitivity error < 0.5%, offsets ≤ 14 mg | Six-orientation test + manual decode of a 12-byte burst |

## 🔗 Detailed Learning Notes

Full experiment logs with waveform screenshots: [Notion Page](https://www.notion.so/Learn-MCU-Schedule-Share-35a47542eba38050b252e5241dd0e5c1)

## ⚙️ Build Environment

- **Toolchain**: mbed-tools CLI + CMake + Ninja + GNU Arm Embedded Toolchain（目前使用 14.2 版）
- **OS**: Mbed OS 6
- **Language**: C / C++
- **Host**: Windows，本地編譯（Keil Studio Cloud 已於 2026/07 關閉服務，改用本地 CLI 工具鏈）
- **Build Script**: [`build-day-from-portfolio.ps1`](./build-day-from-portfolio.ps1) — 自動把指定 Day 的原始碼複製進純 ASCII 路徑的獨立編譯 workspace（Arm GCC 連結器在含中文字路徑下會出錯），編譯完自動偵測並燒錄到板子。需在 **PowerShell** 下執行：
    ```powershell
    powershell -ExecutionPolicy Bypass -File .\build-day-from-portfolio.ps1 -Day Day06_OLED_SPI
    ```