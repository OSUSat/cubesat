/**
 * @file hal_time.c
 * @brief STM32 time HAL implementation.
 */

#include "hal_time.h"
#include "stm32l4xx_hal.h"

void hal_time_init(void) {
    // STM32 HAL already initializes SysTick during HAL_Init()
}

uint32_t hal_time_get_ms(void) { return HAL_GetTick(); }

uint64_t hal_time_get_us(void) {
    // approximate microseconds from millisecond tick
    return (uint64_t)HAL_GetTick() * 1000ULL;
}

void hal_time_delay_ms(uint32_t ms) { HAL_Delay(ms); }
