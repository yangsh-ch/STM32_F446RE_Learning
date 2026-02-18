#include <cstring>
#include "uart_debug.h"

// 強迫將文字從 PA_9 送出
static BufferedSerial serial_port(PA_9, NC, 9600); 

void uart_debug_init() {
    // 這裡可以放 baud rate 或其餘初始化設定
}

void uart_send_log(const char* message) {
    // 計算字串長度並送出
    serial_port.write(message, strlen(message));
}