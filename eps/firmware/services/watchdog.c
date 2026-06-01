/**
 * @file watchdog.c
 * @brief Watchdog Service Implementation.
 */

#include "watchdog.h"
#include "eps_config.h"
#include "hal_gpio.h"
#include "hal_time.h"
#include "iwdg.h"
#include "logging.h"
#include <stddef.h>

void watchdog_init(watchdog_t *watchdog) {
    if (watchdog == NULL) {
        return;
    }

    watchdog->timeout_ms = 512;
    watchdog->last_pet_tick = hal_time_get_ms();
    watchdog->enabled = true;

    // initialize external watchdog pin state
    hal_gpio_write(WATCHDOG_WDI_PIN, HAL_GPIO_STATE_LOW);
}

void watchdog_pet(watchdog_t *watchdog) {
    if (watchdog == NULL || !watchdog->enabled) {
        return;
    }

    if (HAL_IWDG_Refresh(&hiwdg) != HAL_OK) {
        // handle refresh failure by logging an error
        LOG_ERROR(EPS_COMPONENT_MAIN, "watchdog pet failed");
    }
    hal_gpio_toggle(WATCHDOG_WDI_PIN);
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
