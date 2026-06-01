/**
 * @file power_policies.c
 * @brief Application layer for power management state machine.
 */

#include "power_policies.h"
#include "battery_management.h"
#include "events.h"
#include "mppt_controller.h"
#include "osusat/event_bus.h"
#include "redundancy_manager.h"
#include <stdint.h>
#include <string.h>

static void handle_battery_event(const osusat_event_t *e, void *ctx);
static void handle_redundancy_event(const osusat_event_t *e, void *ctx);
static void handle_mppt_event(const osusat_event_t *e, void *ctx);
static void handle_systick_event(const osusat_event_t *e, void *ctx);

void power_policies_init(power_policies_t *app) {
    if (app == NULL) {
        return;
    }

    memset(app, 0, sizeof(power_policies_t));
    app->initialized = true;

    // battery management events
    osusat_event_bus_subscribe(BATTERY_EVENT_CRITICAL_LOW, handle_battery_event,
                               app);
    osusat_event_bus_subscribe(BATTERY_EVENT_FULLY_CHARGED,
                               handle_battery_event, app);

    // redundancy health events
    osusat_event_bus_subscribe(REDUNDANCY_EVENT_SYSTEM_HEALTH_CHANGED,
                               handle_redundancy_event, app);

    // mppt controller events
    osusat_event_bus_subscribe(MPPT_EVENT_FAULT_DETECTED, handle_mppt_event,
                               app);
    osusat_event_bus_subscribe(MPPT_EVENT_PGOOD_CHANGED, handle_mppt_event,
                               app);

    // system tick events
    osusat_event_bus_subscribe(EVENT_SYSTICK, handle_systick_event, app);
}

static void handle_battery_event(const osusat_event_t *e, void *ctx) {
    power_policies_t *app __attribute__((unused)) = (power_policies_t *)ctx;

    switch (e->id) {
    case BATTERY_EVENT_CRITICAL_LOW:
        // on critical battery, request to switch to safe mode
        osusat_event_bus_publish(APP_EVENT_REQUEST_POWER_PROFILE_SAFE, NULL, 0);

        break;

    case BATTERY_EVENT_FULLY_CHARGED:
        // if battery is fully charged, we can request to go back to nominal
        // mode
        //
        // TODO: investigate uptime implications with battery capacity & charge
        // time/windows
        osusat_event_bus_publish(APP_EVENT_REQUEST_POWER_PROFILE_NOMINAL, NULL,
                                 0);

        break;

    default:
        break;
    }
}

static void handle_mppt_event(const osusat_event_t *e, void *ctx) {
    power_policies_t *app = (power_policies_t *)ctx;

    if (app == NULL || !app->initialized) {
        return;
    }

    switch (e->id) {
    case MPPT_EVENT_FAULT_DETECTED:
        if (e->payload_len >= sizeof(uint8_t)) {
            uint8_t failed_channel = e->payload[0];

            if (failed_channel < 6) {
                osusat_event_bus_publish(APP_EVENT_REQUEST_MPPT_DISABLE_CHANNEL,
                                         &failed_channel, sizeof(uint8_t));

                // start cooldown timer for the channel
                app->mppt_cooldown[failed_channel] = 500;
            }
        }
        break;

    case MPPT_EVENT_PGOOD_CHANGED:
        // handle power good state changes
        break;

    default:
        break;
    }
}

static void handle_redundancy_event(const osusat_event_t *e, void *ctx) {
    power_policies_t *app = (power_policies_t *)ctx;

    if (app == NULL || !app->initialized) {
        return;
    }

    if (e->id == REDUNDANCY_EVENT_SYSTEM_HEALTH_CHANGED) {
        if (e->payload_len >= sizeof(system_health_t)) {
            system_health_t health;
            memcpy(&health, e->payload, sizeof(system_health_t));

            if (health == SYSTEM_HEALTH_FAULT) {
                // request transition to safe mode
                osusat_event_bus_publish(APP_EVENT_REQUEST_POWER_PROFILE_SAFE,
                                         NULL, 0);
            } else if (health == SYSTEM_HEALTH_OK) {
                // request transition back to nominal mode
                osusat_event_bus_publish(
                    APP_EVENT_REQUEST_POWER_PROFILE_NOMINAL, NULL, 0);
            }
        }
    }
}

static void handle_systick_event(const osusat_event_t *e, void *ctx) {
    (void)e;
    power_policies_t *app = (power_policies_t *)ctx;

    if (app == NULL || !app->initialized) {
        return;
    }

    for (uint8_t i = 0; i < 6; i++) {
        if (app->mppt_cooldown[i] > 0) {
            app->mppt_cooldown[i]--;
            if (app->mppt_cooldown[i] == 0) {
                // publish enable channel event
                osusat_event_bus_publish(APP_EVENT_REQUEST_MPPT_ENABLE_CHANNEL,
                                         &i, sizeof(uint8_t));
            }
        }
    }
}
