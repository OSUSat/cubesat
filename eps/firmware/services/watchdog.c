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
#include "osusat/event_bus.h"
#include <stddef.h>

/**
 * @brief System Tick Handler.
 * Called automatically by the Event Bus.
 *
 * @param e   The event (SYSTICK).
 * @param ctx The context pointer (points to watchdog_t).
 */
static void watchdog_handle_tick(const osusat_event_t *e, void *ctx);

void watchdog_init(watchdog_t *watchdog) {
    if (watchdog == NULL) {
        return;
    }

    watchdog->timeout_ms = 512;
    watchdog->last_pet_tick = hal_time_get_ms();
    watchdog->enabled = true;

    // initialize watchdog scaling configuration pins
    hal_gpio_write(WATCHDOG_SET0_PIN, HAL_GPIO_STATE_HIGH);
    hal_gpio_write(WATCHDOG_SET1_PIN, HAL_GPIO_STATE_HIGH);

    // initialize external watchdog pin state
    hal_gpio_write(WATCHDOG_WDI_PIN, HAL_GPIO_STATE_LOW);

    osusat_event_bus_subscribe(EVENT_SYSTICK, watchdog_handle_tick, watchdog);
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

static void watchdog_handle_tick(const osusat_event_t *e, void *ctx) {
    (void)e;

    watchdog_t *watchdog = (watchdog_t *)ctx;

    if (watchdog == NULL || !watchdog->enabled) {
        return;
    }

    watchdog_pet(watchdog);
}
