/**
 * @file watchdog.c
 * @brief Watchdog Service Implementation.
 */

#include "watchdog.h"
#include "hal_time.h"
#include "iwdg.h"
#include <stddef.h>

void watchdog_init(watchdog_t *watchdog) {
    if (watchdog == NULL) {
        return;
    }

    watchdog->timeout_ms = 512;
    watchdog->last_pet_tick = hal_time_get_ms();
    watchdog->enabled = true;
}

void watchdog_pet(watchdog_t *watchdog) {
    if (watchdog == NULL) {
        return;
    }

    HAL_IWDG_Refresh(&hiwdg);
    watchdog->last_pet_tick = hal_time_get_ms();
}

void watchdog_force_reset(watchdog_t *watchdog) {
    if (watchdog == NULL) {
        return;
    }

    while (1) {
        // block indefinitely to let hardware watchdog expire
    }
}
