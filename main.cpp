#include "mbed.h"
#include "uart_debug.h"

// 宣告 LED 腳位 (Nucleo 板上的綠色 LED)
DigitalOut led(LED1);

int main() {
    // 1. 初始化所有模組
    uart_debug_init();

    while(1) {
        // 2. 呼叫封裝好的 UART 函數
        uart_send_log("Test\n");
        
        // 3. LED 狀態閃爍
        led = !led;
        
        // 閃爍頻率優化 (500ms)
        ThisThread::sleep_for(500ms); 
    }
}