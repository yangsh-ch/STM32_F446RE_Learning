/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"

DigitalOut led(LED1);
DigitalIn button(BUTTON1); 

// 定義一個變數來記住燈現在應該是開還是關
bool led_status = false; 

int main() {
    printf("Day 2: Toggle System Ready.\n");

    while(1) {
        // 偵測按鈕按下 (0 為按下)
        if (button == 0) {
            // 反轉狀態：如果原本是 false 變成 true，反之亦然
            led_status = !led_status; 
            led = led_status;
            
            // 在 Serial Monitor 輸出目前的狀態
            printf("LED is now %s\n", led_status ? "ON" : "OFF");

            // 重要的「去彈跳」與「等待放開」
            ThisThread::sleep_for(50ms); // 避開機械彈跳
            while(button == 0);          // 如果按著不放，程式會卡在這裡，避免瘋狂切換
            ThisThread::sleep_for(50ms); // 放開後的穩定
        }
    }
}