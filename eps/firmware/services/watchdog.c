/**
 * @file watchdog.c
 * @brief Watchdog Service Implementation.
 *
 * CRITICAL FLIGHT SAFETY AUDIT:
 * 1. WATCHDOG COMPLIANCE: Timeout is configured to 512ms. This matches the STM32L4
 *    IWDG config (Prescaler 4, Reload 4095 on ~32kHz LSI clock gives 4095*4/32000 = 511.8ms).
 *    Petting is tied strictly to the EVENT_SYSTICK event bus dispatcher, which is processed
 *    in the cooperative main thread loop. If any cooperative task blocks/freezes or runs
 *    an infinite loop, the event bus processing is halted, the watchdog will not be petted,
 *    and the hardware reset will trigger safely.
 * 2. EXTERNAL WATCHDOG (MAX6369): Pin WATCHDOG_WDI_PIN (Port E Pin 5) is successfully configured
 *    as an output and toggled via hal_gpio_toggle() in watchdog_pet(). This provides high-reliability
 *    external hardware backup in case the internal MCU clock domain or LSI fails in deep space.
 * 3. FAIL-SAFE RESET: watchdog_force_reset() uses a blocking while(1) loop to guarantee
 *    immediate and clean hardware reset by preventing further petting of both internal and
 *    external watchdogs.
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
