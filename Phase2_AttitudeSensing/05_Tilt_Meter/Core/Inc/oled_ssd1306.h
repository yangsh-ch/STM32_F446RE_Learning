#ifndef OLED_SSD1306_H
#define OLED_SSD1306_H

#include "mbed.h"

void oled_init();
void oled_clear();
void oled_write_command(uint8_t cmd);
void oled_write_data(uint8_t data);
void oled_draw_char(char c, uint8_t col, uint8_t page);
void oled_draw_string(const char* str, uint8_t col, uint8_t page);
void oled_set_spi_frequency(uint32_t hz);  // change SCLK speed without re-running init
// Copy a width x (pages*8) px block to the screen; data[p * width + x], bit 0 = top row of page p
void oled_draw_bitmap(const uint8_t* data, uint8_t col, uint8_t page, uint8_t width, uint8_t pages);

#endif
