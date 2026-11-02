#include "mbed.h"
#include "lsm6ds3.h"

// Streams every IMU sample (104Hz) over UART for tools/attitude_viewer.html; no OLED.
// Same console, I2C and conversions as Phase 2-03/2-04, plus the MCU timestamp so the
// viewer integrates the gyro over the real sample interval instead of browser arrival times.
FileHandle *mbed::mbed_override_console(int) {
    static BufferedSerial console(USBTX, USBRX, 115200);
    return &console;
}

// I2C1: mbed takes (SDA, SCL)
static I2C i2c(PB_9, PB_8);

// +-2g / +-250dps sensitivity (datasheet Rev 10 Table 3): 0.061 mg/LSB, 8.75 mdps/LSB
static int32_t raw_to_mg(int16_t raw)   { return (int32_t)raw * 61 / 1000; }
static int32_t raw_to_mdps(int16_t raw) { return (int32_t)raw * 875 / 100; }

int main() {
    i2c.frequency(100000);
    lsm6ds3_init(i2c);

    bool ok = lsm6ds3_configure();
    printf("\r\n[IMU] config %s\r\n", ok ? "OK" : "FAIL");
    if (!ok) {
        while (true) {
            ThisThread::sleep_for(1s);
        }
    }

    Timer timer;
    timer.start();
    while (true) {
        // No sleep: poll until the next 104Hz sample so none is skipped
        while (!lsm6ds3_data_ready()) {
        }
        uint32_t us = (uint32_t)timer.elapsed_time().count();   // wraps after ~71 min; viewer handles it

        Lsm6ds3Raw raw;
        if (!lsm6ds3_read_raw(&raw)) {
            printf("[IMU] read FAIL\r\n");
            continue;
        }

        // ~65 chars x 104Hz = ~6.8 kB/s, within 115200 baud (~11.5 kB/s)
        printf("T[us] %lu A[mg] %ld %ld %ld G[mdps] %ld %ld %ld\r\n", us,
               raw_to_mg(raw.ax), raw_to_mg(raw.ay), raw_to_mg(raw.az),
               raw_to_mdps(raw.gx), raw_to_mdps(raw.gy), raw_to_mdps(raw.gz));
    }
}
