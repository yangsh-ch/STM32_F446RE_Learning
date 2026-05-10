# STM32 F446RE Firmware Learning Journey

> Hands-on embedded firmware development using STM32F446RE (NUCLEO-F446RE) with Mbed OS.
> 

> Focused on low-level peripheral control, interrupt-driven design, and digital power control — targeting server PSU firmware applications.
> 

## 🎯 Project Goal

Bridge from communication-layer firmware (UART/PCIe/I2C) to **digital power control firmware** relevant to server infrastructure (PSU, data center).

## 🛠 Hardware Setup

| Component | Model | Role |
| --- | --- | --- |
| MCU Board | NUCLEO-F446RE (Cortex-M4 @ 180MHz) | Main controller |
| Logic Analyzer | — | Signal verification & timing measurement |

## 📁 Repository Structure

```
STM32_F446RE_Learning/
├── Day01_02_Blinky_UART/    # GPIO output + Serial debug
├── Day03_GPIO/              # Digital input/output & debounce
├── Day04_UART_Decode/       # UART capture with logic analyzer
├── Day05_Interrupt/         # ISR design, latency measurement (170ms → <50µs)
├── Phase2_PowerCtrl/        # [WIP] ADC + PWM + PID digital power control
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

### 🔄 Phase 2 — Digital Power Control (In Progress)

| Module | Topic | Status |
| --- | --- | --- |
| Day 6 | ADC voltage sampling | 🔄 In progress |
| Day 7 | TIM + PWM output (100kHz) | ⏳ Planned |
| Day 8 | PID closed-loop voltage control | ⏳ Planned |
| Day 9 | PMBus (I2C) — READ_VOUT, STATUS_BYTE | ⏳ Planned |

## 🔬 Key Technical Finding — Day 5

Measured interrupt latency under different conditions:

| Trigger Source | Latency | Root Cause |
| --- | --- | --- |
| Mechanical button (BUTTON1) | 140~200ms | Mechanical bounce + Mbed OS debounce |
| Disable Deep Sleep | ~141ms | Sleep wake-up overhead reduced slightly |
| Digital loopback (D2→D3) | **10~50µs** | Pure hardware ISR, no physical bounce |

> Insight: Mbed OS `InterruptIn` callback goes through RTOS dispatcher. For true µs-level response, direct CMSIS NVIC register control or bare-metal ISR is required.
> 

## 🔗 Detailed Learning Notes

Full experiment logs with waveform screenshots: [Notion Page](https://www.notion.so/Schedule-2fa47542eba3800b8c26da6e06eb93e7?pvs=21)

## ⚙️ Build Environment

- **IDE**: Keil Studio Cloud (Arm Mbed)
- **OS**: Mbed OS 6
- **Language**: C / C++
- **Toolchain**: GNU Arm Embedded Toolchain