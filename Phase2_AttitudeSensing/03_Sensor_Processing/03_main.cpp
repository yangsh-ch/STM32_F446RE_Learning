#include "mbed.h"
#include "lsm6ds3.h"

// Continuous 6-axis log is ~600 chars/s, too close to the 9600-baud default (~960 chars/s),
// so route printf to the ST-LINK VCP at 115200 instead.
FileHandle *mbed::mbed_override_console(int) {
    static BufferedSerial console(USBTX, USBRX, 115200);
    return &console;
}

// I2C1: mbed takes the pins in (SDA, SCL) order
static I2C i2c(PB_9, PB_8);

// Sensitivity at +-2g / +-250dps (datasheet Rev 10, Table 3): 0.061 mg/LSB, 8.75 mdps/LSB.
// minimal-printf has no float support by default, so convert with integer math.
static int32_t raw_to_mg(int16_t raw)   { return (int32_t)raw * 61 / 1000; }
static int32_t raw_to_mdps(int16_t raw) { return (int32_t)raw * 875 / 100; }

int main() {
    i2c.frequency(100000);
    lsm6ds3_init(i2c);

    // Step 1: configure and read back every control register
    printf("\r\n[IMU] config %s\r\n", lsm6ds3_configure() ? "OK" : "FAIL");

    while (true) {
        // Step 2: wait until both accel and gyro have a new sample
        while (!lsm6ds3_data_ready()) {
        }

        // Step 3-5: one 12-byte burst, combine bytes, convert to mg / mdps
        Lsm6ds3Raw raw;
        if (lsm6ds3_read_raw(&raw)) {
            printf("A[mg] %6ld %6ld %6ld  G[mdps] %7ld %7ld %7ld\r\n",
                   raw_to_mg(raw.ax), raw_to_mg(raw.ay), raw_to_mg(raw.az),
                   raw_to_mdps(raw.gx), raw_to_mdps(raw.gy), raw_to_mdps(raw.gz));
        } else {
            printf("[IMU] read FAIL\r\n");
        }

        // Step 6: log at about 10 lines/s
        ThisThread::sleep_for(100ms);
    }
}
