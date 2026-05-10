#include "mbed.h"

DigitalOut led(LED1);
DigitalIn button(BUTTON1); 

// LED state flag
bool led_status = false; 

int main() {
    printf("Day 3: Toggle System Ready.\n");

    while(1) {
        // Detect button press (Active Low)
        if (button == 0) {
            // Toggle state and update output
            led_status = !led_status; 
            led = led_status;
            
            printf("LED is now %s\n", led_status ? "ON" : "OFF");

            // Debounce and wait for release
            ThisThread::sleep_for(50ms); // Debounce delay to ignore mechanical noise
            while(button == 0);          // Wait for button release to prevent unintended re-triggering
            ThisThread::sleep_for(50ms); // Post-release stabilization delay
        }
    }
}