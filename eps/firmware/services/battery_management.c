/**
 * @file battery_management.c
 * @brief Battery Management Service Implementation
 */

#include "battery_management.h"
#include "osusat/event_bus.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// TODO: ensure system tick fires every 100Hz (10ms)
// this way we can update the battery loop every 100ms
#define BATTERY_UPDATE_INTERVAL_TICKS 10
#define TELEMETRY_INTERVAL_CYCLES 600

/**
 * @brief System Tick Handler.
 * Called automatically by the Event Bus.
 *
 * @param e   The event (SYSTICK).
 * @param ctx The context pointer (points to battery_management_t).
 */
static void battery_handle_tick(const osusat_event_t *e, void *ctx);

/**
 * @brief Internal update logic (reads sensors).
 */
static void battery_perform_update(battery_management_t *manager);

/**
 * @brief Hardware self-check (I2C comms, initial voltage).
 */
static bool battery_run_diagnostics(battery_management_t *manager);

void battery_init(battery_management_t *manager) {
    if (manager == NULL) {
        return;
    }

    memset(manager, 0, sizeof(battery_management_t));

    bool healthy = battery_run_diagnostics(manager);

    if (healthy) {
        manager->initialized = true;
        osusat_event_bus_publish(BATTERY_EVENT_SELF_CHECK_PASSED, NULL, 0);
    } else {
        manager->initialized = false;

        uint8_t failure_code = 0x01; // TODO: replace with real failure code
        osusat_event_bus_publish(BATTERY_EVENT_SELF_CHECK_FAILED, &failure_code,
                                 1);
    }

    osusat_event_bus_subscribe(EVENT_SYSTICK, battery_handle_tick, manager);
}

void battery_charge_control(battery_management_t *manager, bool enable) {
    if (!manager || !manager->initialized)
        return;

    manager->battery_status.charging = enable;

    // TODO: perform hardware action

    osusat_event_bus_publish(BATTERY_EVENT_CHARGING_CHANGE, &enable, 1);
}

void battery_protect_mode(battery_management_t *manager) {
    if (!manager)
        return;

    // TODO: figure out functionality

    battery_charge_control(manager, false);

    manager->battery_status.protection = true;

    osusat_event_bus_publish(BATTERY_EVENT_FAULT_DETECTED,
                             &manager->battery_status,
                             sizeof(battery_status_t));
}

static void battery_handle_tick(const osusat_event_t *e, void *ctx) {
    (void)e;

    battery_management_t *manager = (battery_management_t *)ctx;

    if (manager == NULL || !manager->initialized) {
        return;
    }

    static uint32_t tick_counter =
        0; // prescaler for running logic at 10Hz instead of system tick rate
    tick_counter++;

    if (tick_counter >= BATTERY_UPDATE_INTERVAL_TICKS) {
        tick_counter = 0;
        battery_perform_update(manager);
    }
}

static void battery_perform_update(battery_management_t *manager) {
    float voltage = 0; // TODO: replace with real read

    manager->battery_status.voltage = voltage;

    const float CRITICAL_THRESHOLD = 3.3f;

    if (voltage < CRITICAL_THRESHOLD && !manager->battery_status.protection) {
        battery_protect_mode(manager);

        osusat_event_bus_publish(BATTERY_EVENT_CRITICAL_LOW, &voltage,
                                 sizeof(float));
    }

    static uint32_t tick_counter = 0;
    tick_counter++;

    if (tick_counter >= TELEMETRY_INTERVAL_CYCLES) {
        tick_counter = 0;

        osusat_event_bus_publish(BATTERY_EVENT_TELEMETRY,
                                 &manager->battery_status,
                                 sizeof(battery_status_t));
    }
}

static bool battery_run_diagnostics(battery_management_t *manager) {
    (void)manager;

    // TODO: perform real checks

    return true;
}
