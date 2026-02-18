#include "exti_control.h"

// 宣告內部使用的硬體物件
static DigitalOut trigger(D2);    // 模擬按鈕
static InterruptIn pulse_in(D3);  // 中斷入口
static DigitalOut led(LED1);      // 狀態顯示

// 中斷服務程式 (ISR)
void on_pulse() {
    led = !led; 
}

void exti_init() {
    trigger = 1;                // 確保初始為高電位
    pulse_in.fall(&on_pulse);   // 設定 D3 偵測下降緣
}

void exti_generate_trigger() {
    trigger = 0;                // 產生下降緣
    wait_us(10);                // 維持低電位
    trigger = 1;                // 回復高電位
}