#include "time.h"
#include "stm32l432xx.h"

// timer values
volatile unsigned int int_cnt = 0;

// This function MUST be named SysTick_Handler for the CMSIS framework
// code to link to it correctly.
void SysTick_Handler(void) {
    int_cnt++;
}

void SysTick_Init(void) {
    SysTick->CTRL = 0;
    SysTick->LOAD = 3999; // every 4000 cycles (4MHz / 4000 = 1kHz)
    // This sets the priority of the interrupt to 15 (2^4 - 1), which is the
    // largest supported value (aka lowest priority)
    NVIC_SetPriority(SysTick_IRQn, (1<<__NVIC_PRIO_BITS) - 1);
    SysTick->VAL = 0;
    SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk;
    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
}

uint32_t get_time(void) {
    return int_cnt;
}

void reset_time(void) {
    int_cnt = 0;
}