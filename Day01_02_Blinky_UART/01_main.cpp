#include "mbed.h"

// Initialize digital output for LED1 (PA_5)
DigitalOut myled(LED1);

int main() {
    printf("Hello World! STM32F446RE is running.\n");

    while(1) {
        myled = 1;   			// Turn LED on
        thread_sleep_for(500);	// 500ms delay
        myled = 0;				// Turn LED off
        thread_sleep_for(500);	
    }
}