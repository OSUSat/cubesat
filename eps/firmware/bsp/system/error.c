#include "error.h"
#include "stm32l496xx.h"

__attribute__((noreturn)) void bsp_error_trap(void) {
    __disable_irq();
    while (1) {
        // optionally kick external watchdog
    }
}
