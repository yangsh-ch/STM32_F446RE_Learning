#include "mbed.h"
#include "exti_control.h"

int main() {
    // Initialization
    exti_init();

    while(1) {
        // Execute hardware trigger every 500ms
        exti_generate_trigger();
        
        ThisThread::sleep_for(500ms);
    }
}