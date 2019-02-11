#ifndef __MAIN__
#define __MAIN__

#include "stm32f0xx_hal.h"

#define DEBUG_WITH_USART 1

extern UART_HandleTypeDef huart1;

#if 1 //DEBUG_WITH_USART
    void print_ts(char *str);
    void printls_int(uint32_t i);
    #define printls(c) print_usart(c)
    void printls(char *str);
#else
    #define printls(c)
    #define printls_int(c)
#endif

char* itoa(int32_t i, char b[]);
int atoi(const char *str);

#endif
