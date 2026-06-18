/**
 * @file battery_management.c
 * @brief Battery Management Service Implementation
 */

#include "battery_management.h"
#include "eps_config.h"
#include "logging.h"
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
        LOG_INFO(EPS_COMPONENT_MAIN,
                 "Battery management initialized successfully");
        osusat_event_bus_publish(BATTERY_EVENT_SELF_CHECK_PASSED, NULL, 0);
    } else {
        manager->initialized = false;
        LOG_ERROR(EPS_COMPONENT_MAIN,
                  "Battery self-check failed on initialization");

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

    LOG_INFO(EPS_COMPONENT_MAIN, "Battery charge control state set to %d",
             enable);
    // TODO: perform hardware action

    osusat_event_bus_publish(BATTERY_EVENT_CHARGING_CHANGE, &enable, 1);
}

void battery_protect_mode(battery_management_t *manager) {
    if (!manager)
        return;

    // TODO: figure out functionality

    battery_charge_control(manager, false);

    manager->battery_status.protection = true;
    LOG_ERROR(EPS_COMPONENT_MAIN,
              "Battery protection mode triggered! Voltage: %.2fV",
              manager->battery_status.voltage);

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

    manager->tick_counter++;

    if (manager->tick_counter >= BATTERY_UPDATE_INTERVAL_TICKS) {
        manager->tick_counter = 0;
        battery_perform_update(manager);
    }
}

static void battery_perform_update(battery_management_t *manager) {
    float voltage = 3.8f; // Mock nominal battery voltage for testing

    manager->battery_status.voltage = voltage;

    if (voltage < CRITICAL_BATTERY_VOLTAGE_THRESHOLD &&
        !manager->battery_status.protection) {
        LOG_WARN(EPS_COMPONENT_MAIN,
                 "Critical low battery voltage detected: %.2fV", voltage);
        battery_protect_mode(manager);

        osusat_event_bus_publish(BATTERY_EVENT_CRITICAL_LOW, &voltage,
                                 sizeof(float));
    }

    manager->telemetry_tick_counter++;

    if (manager->telemetry_tick_counter >= TELEMETRY_INTERVAL_CYCLES) {
        manager->telemetry_tick_counter = 0;
        LOG_INFO(
            EPS_COMPONENT_MAIN, "Battery update telemetry: V=%.2fV, I=%.2fA",
            manager->battery_status.voltage, manager->battery_status.current);

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
