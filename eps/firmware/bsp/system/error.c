#include "error.h"
#include "stm32l496xx.h"

__attribute__((noreturn)) void bsp_error_trap(void) {
    __disable_irq();
    while (1) {
        // optionally kick external watchdog
    }
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
    __disable_irq();
    while (1) {
        // optional: blink an LED or log error to persistent storage
    }
}
