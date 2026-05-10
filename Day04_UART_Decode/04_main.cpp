#include "mbed.h"
#include "uart_debug.h"

// Define LED pin (Green LED on Nucleo)
DigitalOut led(LED1);

int main() {
    // Initialize modules
    uart_debug_init();

    while(1) {
        // Send log via UART wrapper
        uart_send_log("Test\n");
        
        // Toggle LED state
        led = !led;
        
        // 500ms delay
        ThisThread::sleep_for(500ms); 
    }
}