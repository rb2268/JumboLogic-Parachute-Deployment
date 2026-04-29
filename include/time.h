#ifndef TIME_H
#define TIME_H

#include <stdint.h>

void SysTick_Handler(void);
void SysTick_Init(void);

uint32_t get_time(void);
void reset_time(void);

#endif