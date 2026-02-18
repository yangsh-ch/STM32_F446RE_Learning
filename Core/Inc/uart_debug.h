#ifndef UART_DEBUG_H
#define UART_DEBUG_H

#include "mbed.h"

// 初始化 UART 腳位 (PA_9)
void uart_debug_init();

// 封裝發送文字的函數，讓呼叫更簡單
void uart_send_log(const char* message);

#endif