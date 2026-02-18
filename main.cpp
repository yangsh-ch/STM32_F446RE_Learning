#include "mbed.h"
#include "uart_debug.h"    // Day 4
#include "exti_control.h"  // Day 5

int main() {
    // 初始化
    //uart_debug_init();
    exti_init();

    while(1) {

        // 每 500ms 執行一次硬體觸發
        exti_generate_trigger();
        
        ThisThread::sleep_for(500ms);
    }
}