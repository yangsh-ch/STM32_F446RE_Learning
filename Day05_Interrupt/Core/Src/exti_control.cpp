#include "exti_control.h"

// Internal hardware objects
static DigitalOut trigger(D2);    // Simulated button
static InterruptIn pulse_in(D3);  // Interrupt input
static DigitalOut led(LED1);      // Status indicator

// Interrupt Service Routine (ISR)
void on_pulse() {
    led = !led; 
}

void exti_init() {
    trigger = 1;                // Ensure initial high state
    pulse_in.fall(&on_pulse);   // Attach falling edge trigger to D3
}

void exti_generate_trigger() {
    trigger = 0;                // Pull low (falling edge)
    wait_us(10);                // Maintain pulse width
    trigger = 1;                // Return to high
}