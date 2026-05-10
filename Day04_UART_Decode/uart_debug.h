#ifndef UART_DEBUG_H
#define UART_DEBUG_H

#include "mbed.h"

// Init UART (PA_9)
void uart_debug_init();

// Wrapper for simplified logging
void uart_send_log(const char* message);

#endif