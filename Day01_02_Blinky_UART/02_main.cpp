#include "mbed.h"

// Initialize buffered serial object with a baud rate of 115200
static BufferedSerial pc(USBTX, USBRX, 115200);

DigitalOut myled(LED1);

int main() {
    // Serial config note
    printf("Testing Serial...\n");
    
    while(1) {
        myled = !myled;					// Toggle LED state
        printf("LED state changed!\n");
        ThisThread::sleep_for(1000ms);	// 1s delay
    }
}