#include <cstring>
#include "uart_debug.h"

// Force TX on PA_9
static BufferedSerial serial_port(PA_9, NC, 9600); 

void uart_debug_init() {
    // Baud rate or peripheral init
}

void uart_send_log(const char* message) {
    // Write message to UART
    serial_port.write(message, strlen(message));
}