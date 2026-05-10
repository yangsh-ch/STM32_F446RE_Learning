#ifndef EXTI_CONTROL_H
#define EXTI_CONTROL_H

#include "mbed.h"

// Init EXTI: Set D2 as trigger source, D3 as interrupt input
void exti_init();

// Generate a falling edge trigger pulse
void exti_generate_trigger();

#endif