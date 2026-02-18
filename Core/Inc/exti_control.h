#ifndef EXTI_CONTROL_H
#define EXTI_CONTROL_H

#include "mbed.h"

// 初始化中斷模組：設定 D2 為觸發源, D3 為中斷輸入
void exti_init();

// 產生一個下降緣觸發脈衝
void exti_generate_trigger();

#endif