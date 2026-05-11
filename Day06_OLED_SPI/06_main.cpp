#include "mbed.h"
#include "oled_ssd1306.h"

int main() {
    oled_init();
    oled_clear();

    oled_draw_string("Hello STM32!", 0, 0);
    oled_draw_string("Day 6 Done!", 0, 2);
    oled_draw_string("SPI @ 1MHz",  0, 4);

    while (1) ThisThread::sleep_for(1s);
}