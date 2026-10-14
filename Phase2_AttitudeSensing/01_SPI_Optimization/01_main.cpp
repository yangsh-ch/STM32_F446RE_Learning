#include "mbed.h"
#include "oled_ssd1306.h"

// Force TX on PA_9 (same convention as Day04_UART_Decode) so it can be probed
// directly by an external logic analyzer, instead of USBTX (PA_2 on this board,
// routed through the ST-LINK VCP, not easily wired to a spare analyzer channel).
static BufferedSerial pc(PA_9, NC, 115200);

// SSD1306 datasheet (Table 13-4, 4-wire SPI): tCYCLE min = 100ns -> 10MHz is the
// chip's own ceiling under ideal signal conditions. 16MHz deliberately exceeds
// that spec to find where dupont-wire signal integrity actually breaks down.
static const uint32_t test_freqs_hz[] = {1000000, 4000000, 8000000, 16000000};
static const int NUM_FREQS = sizeof(test_freqs_hz) / sizeof(test_freqs_hz[0]);

int main() {
    oled_init();
    int cycle = 0;

    while (1) {
        for (int i = 0; i < NUM_FREQS; i++) {
            uint32_t hz = test_freqs_hz[i];
            oled_set_spi_frequency(hz);

            oled_clear();
            char line[32];
            oled_draw_string("SPI Freq Test", 0, 0);
            snprintf(line, sizeof(line), "Now: %lu MHz", hz / 1000000);
            oled_draw_string(line, 0, 2);
            oled_draw_string("Hello STM32!", 0, 4); // fixed reference string: eyeball this for corruption
            snprintf(line, sizeof(line), "Cycle: %d", cycle);
            oled_draw_string(line, 0, 6);

            int len = snprintf(line, sizeof(line), "[SPI] %lu Hz, cycle %d\r\n", hz, cycle);
            pc.write(line, len); // correlate with logic analyzer capture timing

            ThisThread::sleep_for(5s);
        }
        cycle++;
    }
}
