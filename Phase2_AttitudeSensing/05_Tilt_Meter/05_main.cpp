#include "mbed.h"
#include "lsm6ds3.h"
#include "oled_ssd1306.h"
#include <math.h>
#include <string.h>

// Same console as Phase 2-04: the UART log is kept so OLED values can be cross-checked
FileHandle *mbed::mbed_override_console(int) {
    static BufferedSerial console(USBTX, USBRX, 115200);
    return &console;
}

// I2C1: mbed takes the pins in (SDA, SCL) order
static I2C i2c(PB_9, PB_8);

// USER button (PC13): pressing it makes the current pose the new 0 deg
static InterruptIn zero_button(BUTTON1);
static volatile bool zero_requested = false;

// ISR only raises a flag; the angles are read and printed in the main loop
static void on_zero_button() {
    zero_requested = true;
}

// +-2g sensitivity (datasheet Rev 10, Table 3): 0.061 mg/LSB
static int32_t raw_to_mg(int16_t raw) { return (int32_t)raw * 61 / 1000; }

// 8MHz setting = 5.63MHz actual, the fastest one inside the SSD1306 t_cycle spec (Phase 2-01)
static const uint32_t OLED_SPI_HZ = 8000000;

static const float RAD_TO_DEG = 57.2957795f;

// ---- Screen layout: text on the left 64 px, bubble level on the right 64 px ----
static const int TEXT_COLS = 10;              // 10 chars x 6 px = 60 px (cols 0-59)
static const int LEVEL_COL = 64;              // level area: cols 64-127, pages 0-7
static const int LEVEL_SIZE = 64;             // 64 x 64 px
static const int LEVEL_PAGES = LEVEL_SIZE / 8;

// Level geometry, in pixels inside the 64 x 64 area
static const int CX = 32, CY = 32;            // center
static const int OUTER_R = 30;                // outer circle = full scale
static const int BUBBLE_R = 4;
static const int MAX_OFFSET = OUTER_R - BUBBLE_R;     // bubble touches the outer circle
static const float FULL_SCALE_DEG = 30.0f;
static const float PX_PER_DEG = MAX_OFFSET / FULL_SCALE_DEG;   // 26 px / 30 deg
static const int RING_R = 9;                  // inner ring = 10 deg (10 * 0.867 = 8.7 px)

// Frame buffer for the level area only (512 bytes), same byte layout as the
// SSD1306 RAM: byte [page * 64 + x], bit n = row (page * 8 + n)
static uint8_t level_fb[LEVEL_PAGES * LEVEL_SIZE];

static void fb_set_pixel(int x, int y) {
    if (x < 0 || x >= LEVEL_SIZE || y < 0 || y >= LEVEL_SIZE) return;
    level_fb[(y / 8) * LEVEL_SIZE + x] |= (uint8_t)(1 << (y % 8));
}

// Midpoint circle: walk one 1/8 arc and mirror it to the other seven
static void fb_draw_circle(int cx, int cy, int r) {
    int x = r, y = 0, err = 1 - r;
    while (x >= y) {
        fb_set_pixel(cx + x, cy + y); fb_set_pixel(cx - x, cy + y);
        fb_set_pixel(cx + x, cy - y); fb_set_pixel(cx - x, cy - y);
        fb_set_pixel(cx + y, cy + x); fb_set_pixel(cx - y, cy + x);
        fb_set_pixel(cx + y, cy - x); fb_set_pixel(cx - y, cy - x);
        y++;
        if (err < 0) {
            err += 2 * y + 1;
        } else {
            x--;
            err += 2 * (y - x) + 1;
        }
    }
}

// r*r + r instead of r*r gives a rounder disc on small radii
static void fb_fill_circle(int cx, int cy, int r) {
    for (int dy = -r; dy <= r; dy++)
        for (int dx = -r; dx <= r; dx++)
            if (dx * dx + dy * dy <= r * r + r) fb_set_pixel(cx + dx, cy + dy);
}

// Redraw the whole level: scale, then the bubble on top, then send 512 bytes.
// The module lies with +X toward the top of the OLED, so +Y points left
// (right-handed, Z up). A real bubble floats to the HIGH side:
//   +X end raised -> ax > 0 -> pitch < 0 -> bubble moves up (screen y grows downward)
//   +Y end raised -> ay > 0 -> roll  > 0 -> bubble moves left
static void draw_level(float roll, float pitch) {
    memset(level_fb, 0, sizeof(level_fb));

    fb_draw_circle(CX, CY, OUTER_R);
    fb_draw_circle(CX, CY, RING_R);
    for (int i = OUTER_R - 3; i < OUTER_R; i++) {     // 3 px ticks at the 4 axis ends
        fb_set_pixel(CX + i, CY); fb_set_pixel(CX - i, CY);
        fb_set_pixel(CX, CY + i); fb_set_pixel(CX, CY - i);
    }

    float bx = -roll * PX_PER_DEG;
    float by = pitch * PX_PER_DEG;
    float dist = sqrtf(bx * bx + by * by);
    if (dist > MAX_OFFSET) {              // past full scale: stop at the edge, keep the direction
        bx *= MAX_OFFSET / dist;
        by *= MAX_OFFSET / dist;
    }
    fb_fill_circle(CX + (int)lroundf(bx), CY + (int)lroundf(by), BUBBLE_R);

    oled_draw_bitmap(level_fb, LEVEL_COL, 0, LEVEL_SIZE, LEVEL_PAGES);
}

// Tilt from gravity only (same formulas as tools/attitude_viewer.html mode A).
// atan2 only needs the ratio, so raw counts work without converting to mg.
static void raw_to_tilt(const Lsm6ds3Raw &raw, float *roll, float *pitch) {
    float ax = raw.ax, ay = raw.ay, az = raw.az;
    *roll  = atan2f(ay, az) * RAD_TO_DEG;
    *pitch = atan2f(-ax, sqrtf(ay * ay + az * az)) * RAD_TO_DEG;
}

// Keep roll inside -180..180 after subtracting the zero reference
static float wrap_180(float deg) {
    if (deg > 180.0f)   deg -= 360.0f;
    if (deg <= -180.0f) deg += 360.0f;
    return deg;
}

// deg -> "-12.3" style text with one decimal.
// minimal-printf has no %f, so print it as integer tenths.
static void format_deg(char *out, size_t size, float deg) {
    int32_t tenths = (int32_t)lroundf(deg * 10.0f);
    int32_t abs_tenths = tenths < 0 ? -tenths : tenths;
    snprintf(out, size, "%s%ld.%ld", tenths < 0 ? "-" : "", abs_tenths / 10, abs_tenths % 10);
}

// One 10-char row on the left half, e.g. "R    -12.3": label at col 0, angle
// right-aligned on the last col. minimal-printf has no width for %s, so align by hand.
// Always drawing the whole row overwrites leftovers when a number gets shorter.
static void draw_angle_row(uint8_t page, const char *label, float deg) {
    char row[TEXT_COLS + 1];
    char field[12];
    memset(row, ' ', TEXT_COLS);
    row[TEXT_COLS] = '\0';

    memcpy(row, label, strlen(label));
    format_deg(field, sizeof(field), deg);
    int len = strlen(field);              // up to 6 chars: "-180.0"
    memcpy(&row[TEXT_COLS - len], field, len);

    oled_draw_string(row, 0, page);
}

// Zero reference: only changes when the button is pressed, so drawn only then
static void draw_ref(float roll0, float pitch0) {
    draw_angle_row(6, "R", roll0);
    draw_angle_row(7, "P", pitch0);
}

int main() {
    oled_init();                          // runs at 1MHz during reset/init
    oled_set_spi_frequency(OLED_SPI_HZ);
    oled_clear();

    i2c.frequency(100000);
    lsm6ds3_init(i2c);

    bool ok = lsm6ds3_configure();
    printf("\r\n[TILT] config %s\r\n", ok ? "OK" : "FAIL");
    if (!ok) {
        oled_draw_string("IMU config FAIL", 0, 0);
        while (true) {
            ThisThread::sleep_for(1s);
        }
    }
    oled_draw_string("TILT [deg]", 0, 0);   // header drawn once
    oled_draw_string("Ref (USER)", 0, 5);

    float roll0 = 0.0f, pitch0 = 0.0f;    // zero reference, 0 until the button is pressed
    draw_ref(roll0, pitch0);

    zero_button.fall(&on_zero_button);    // button pulls PC13 low when pressed

    while (true) {
        while (!lsm6ds3_data_ready()) {
        }

        Lsm6ds3Raw raw;
        if (!lsm6ds3_read_raw(&raw)) {
            printf("[TILT] read FAIL\r\n");
            ThisThread::sleep_for(100ms);
            continue;
        }

        float roll_abs, pitch_abs;
        raw_to_tilt(raw, &roll_abs, &pitch_abs);

        if (zero_requested) {
            zero_requested = false;
            roll0 = roll_abs;
            pitch0 = pitch_abs;
            char r[12], p[12];
            format_deg(r, sizeof(r), roll0);
            format_deg(p, sizeof(p), pitch0);
            printf("[TILT] zero set: roll %s pitch %s\r\n", r, p);
            draw_ref(roll0, pitch0);
        }

        float roll  = wrap_180(roll_abs - roll0);
        float pitch = pitch_abs - pitch0;   // pitch stays within -90..90, so the difference fits in -180..180

        char r[12], p[12];
        format_deg(r, sizeof(r), roll);
        format_deg(p, sizeof(p), pitch);
        printf("Roll %s Pitch %s  A[mg] %ld %ld %ld\r\n", r, p,
               raw_to_mg(raw.ax), raw_to_mg(raw.ay), raw_to_mg(raw.az));

        draw_angle_row(2, "R", roll);
        draw_angle_row(3, "P", pitch);
        draw_level(roll, pitch);

        // 50 ms (2-04 used 100 ms) so the bubble moves more smoothly; the idle gap
        // stays above the 20 ms that 04 tools/timing.py uses to split loops
        ThisThread::sleep_for(50ms);
    }
}
