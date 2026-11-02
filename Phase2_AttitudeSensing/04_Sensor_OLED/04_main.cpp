#include "mbed.h"
#include "lsm6ds3.h"
#include "oled_ssd1306.h"
#include <string.h>

// Same console as Phase 2-03: the UART log is kept so OLED values can be cross-checked
FileHandle *mbed::mbed_override_console(int) {
    static BufferedSerial console(USBTX, USBRX, 115200);
    return &console;
}

// I2C1: mbed takes the pins in (SDA, SCL) order
static I2C i2c(PB_9, PB_8);

// Sensitivity at +-2g / +-250dps (datasheet Rev 10, Table 3): 0.061 mg/LSB, 8.75 mdps/LSB.
static int32_t raw_to_mg(int16_t raw)   { return (int32_t)raw * 61 / 1000; }
static int32_t raw_to_mdps(int16_t raw) { return (int32_t)raw * 875 / 100; }

// 8MHz setting = 5.63MHz actual, the fastest one inside the SSD1306 t_cycle spec (Phase 2-01)
static const uint32_t OLED_SPI_HZ = 8000000;

// 128px / 6px per char = 21 chars per row
static const int OLED_COLS = 21;

// Copy s into the row so that its last char lands on end_col (right-aligned).
// minimal-printf supports width for integers but not for %s, so align by hand.
static void put_right(char *row, int end_col, const char *s) {
    int len = strlen(s);
    memcpy(&row[end_col - len + 1], s, len);
}

// mdps -> "-2.6" style dps text with one decimal, using integer math only
static void format_dps(char *out, size_t size, int32_t mdps) {
    int32_t tenths = mdps / 100;          // truncates toward 0, like raw_to_mdps()
    int32_t abs_tenths = tenths < 0 ? -tenths : tenths;
    snprintf(out, size, "%s%ld.%ld", tenths < 0 ? "-" : "", abs_tenths / 10, abs_tenths % 10);
}

// One full 21-char row, e.g. "X    31   X   2.2". Always drawing the whole row
// overwrites leftovers when a number gets shorter (e.g. -1000 -> 12).
static void draw_axis_row(uint8_t page, char axis, int32_t mg, int32_t mdps) {
    char row[OLED_COLS + 1];
    char field[12];
    memset(row, ' ', OLED_COLS);
    row[OLED_COLS] = '\0';

    row[0] = axis;
    snprintf(field, sizeof(field), "%ld", mg);
    put_right(row, 6, field);             // accel: cols 2-6, up to "-1999"

    row[10] = axis;
    format_dps(field, sizeof(field), mdps);
    put_right(row, 17, field);            // gyro: cols 12-17, up to "-286.7"

    oled_draw_string(row, 0, page);
}

int main() {
    oled_init();                          // runs at 1MHz during reset/init
    oled_set_spi_frequency(OLED_SPI_HZ);
    oled_clear();

    i2c.frequency(100000);
    lsm6ds3_init(i2c);

    bool ok = lsm6ds3_configure();
    printf("\r\n[IMU] config %s\r\n", ok ? "OK" : "FAIL");
    if (!ok) {
        oled_draw_string("IMU config FAIL", 0, 0);
        while (true) {
            ThisThread::sleep_for(1s);
        }
    }
    oled_draw_string("ACC[mg]   GYR[dps]", 0, 0);   // header drawn once

    uint32_t frame = 0;
    while (true) {
        while (!lsm6ds3_data_ready()) {
        }

        Lsm6ds3Raw raw;
        if (!lsm6ds3_read_raw(&raw)) {
            printf("[IMU] read FAIL\r\n");
            ThisThread::sleep_for(100ms);
            continue;
        }

        int32_t ax = raw_to_mg(raw.ax),   ay = raw_to_mg(raw.ay),   az = raw_to_mg(raw.az);
        int32_t gx = raw_to_mdps(raw.gx), gy = raw_to_mdps(raw.gy), gz = raw_to_mdps(raw.gz);

        printf("A[mg] %6ld %6ld %6ld  G[mdps] %7ld %7ld %7ld\r\n", ax, ay, az, gx, gy, gz);

        draw_axis_row(2, 'X', ax, gx);
        draw_axis_row(3, 'Y', ay, gy);
        draw_axis_row(4, 'Z', az, gz);

        char row[OLED_COLS + 1];
        snprintf(row, sizeof(row), "n=%lu", frame++);
        oled_draw_string(row, 0, 6);      // the counter only grows, so no leftovers

        ThisThread::sleep_for(100ms);
    }
}
