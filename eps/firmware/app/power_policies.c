/**
 * @file power_policies.c
 * @brief Application layer for power management state machine.
 */

#include "power_policies.h"
#include "battery_management.h"
#include "events.h"
#include "osusat/event_bus.h"
#include "redundancy_manager.h"
#include <string.h>

static void handle_battery_event(const osusat_event_t *e, void *ctx);
static void handle_redundancy_event(const osusat_event_t *e, void *ctx);

void power_policies_init(power_policies_t *app) {
    if (app == NULL) {
        return;
    }

    memset(app, 0, sizeof(power_policies_t));
    app->initialized = true;

    // subscribe to events from other services
    osusat_event_bus_subscribe(BATTERY_EVENT_CRITICAL_LOW, handle_battery_event,
                               app);
    osusat_event_bus_subscribe(BATTERY_EVENT_FULLY_CHARGED,
                               handle_battery_event, app);
    // osusat_event_bus_subscribe(REDUNDANCY_EVENT_SYSTEM_HEALTH_CHANGED,
    // handle_redundancy_event, app);
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

static void handle_redundancy_event(const osusat_event_t *e, void *ctx) {
    power_policies_t *app = (power_policies_t *)ctx;

    // TODO: define system health event from redundancy manager.
    //
    // if (e->id == REDUNDANCY_EVENT_SYSTEM_HEALTH_CHANGED) {
    //     system_health_t *health = (system_health_t *)e->payload;
    //
    //     if (*health == SYSTEM_HEALTH_FAULT) {
    //
    //         // on system fault, switch to safe mode
    //         osusat_event_bus_publish(APP_EVENT_REQUEST_POWER_PROFILE_SAFE,
    //         NULL, 0);
    //
    //     } else if (*health == SYSTEM_HEALTH_OK) {
    //
    //         // if system is healthy, go back to nominal mode
    //         osusat_event_bus_publish(APP_EVENT_REQUEST_POWER_PROFILE_NOMINAL,
    //         NULL, 0);
    //     }
    // }
}
