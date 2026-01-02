#include "rail_controller.h"
#include "eps_config.h"
#include "events.h"
#include "osusat/event_bus.h"
#include <stdio.h>
#include <string.h>

#define RAIL_CONTROLLER_UPDATE_INTERVAL_TICKS 10
#define TELEMETRY_INTERVAL_CYCLES 600

/**
 * @brief System Tick Handler.
 * Called automatically by the Event Bus.
 *
 * @param e   The event (SYSTICK).
 * @param ctx The context pointer (points to rail_controller_t).
 */
static void rail_controller_handle_tick(const osusat_event_t *e, void *ctx);

/**
 * @brief Internal update logic (reads sensors, updates state).
 */
static void rail_controller_handle_update(rail_controller_t *manager);

/**
 * @brief Event handler for when a new request is received from the application
 */
static void handle_profile_request_event(const osusat_event_t *e, void *ctx);

void rail_controller_init(rail_controller_t *manager) {
    if (manager == NULL) {
        return;
    }

    memset(manager, 0, sizeof(rail_controller_t));

    for (size_t rail = 0; rail < NUM_POWER_RAILS; rail++) {
        manager->rails[rail].rail_id = (power_rail_t)rail;
        manager->rails[rail].status = RAIL_STATUS_DISABLED;
        manager->rails[rail].enabled = false;
    }

    // TODO: bringup?

    manager->initialized = true;
    osusat_event_bus_subscribe(EVENT_SYSTICK, rail_controller_handle_tick,
                               manager);

    osusat_event_bus_subscribe(APP_EVENT_REQUEST_RAIL_CONTROLLER_ENABLE_RAIL,
                               handle_profile_request_event, manager);
    osusat_event_bus_subscribe(APP_EVENT_REQUEST_RAIL_CONTROLLER_DISABLE_RAIL,
                               handle_profile_request_event, manager);
}

static void handle_profile_request_event(const osusat_event_t *e, void *ctx) {
    rail_controller_t *manager = (rail_controller_t *)ctx;

    if (e->payload_len != 1) {
        return;
    }

    switch (e->id) {
    case APP_EVENT_REQUEST_RAIL_CONTROLLER_ENABLE_RAIL:
        rail_controller_enable(manager, e->payload[0]);
        break;

    case APP_EVENT_REQUEST_RAIL_CONTROLLER_DISABLE_RAIL:
        rail_controller_disable(manager, e->payload[0]);
        break;

    default:
        return; // unknown event
    }
}

void rail_controller_enable(rail_controller_t *controller, power_rail_t rail) {
    (void)controller;
    (void)rail;
    printf("STUB: rail_controller_enable for rail %d\n", rail);
}

void rail_controller_disable(rail_controller_t *controller, power_rail_t rail) {
    (void)controller;
    (void)rail;
    printf("STUB: rail_controller_disable for rail %d\n", rail);
}

static void rail_controller_handle_tick(const osusat_event_t *e, void *ctx) {
    rail_controller_t *manager = (rail_controller_t *)ctx;

    if (manager == NULL || !manager->initialized) {
        return;
    }

    manager->tick_counter++;

    if (manager->tick_counter >= RAIL_CONTROLLER_UPDATE_INTERVAL_TICKS) {
        manager->tick_counter = 0;
        rail_controller_handle_update(manager);
    }

    manager->telemetry_tick_counter++;

    if (manager->telemetry_tick_counter >= TELEMETRY_INTERVAL_CYCLES) {
        manager->telemetry_tick_counter = 0;

        for (size_t rail = 0; rail < NUM_POWER_RAILS; rail++) {
            osusat_event_bus_publish(RAIL_CONTROLLER_TELEMETRY,
                                     &manager->rails[rail], sizeof(rail_t));
        }
    }
}

void rail_controller_handle_update(rail_controller_t *manager) {
    for (size_t rail = 0; rail < NUM_POWER_RAILS; rail++) {
        // TODO: read real sensors per rail
        float voltage = 0.0f;
        float current = 0.0f;

        manager->rails[rail].current = voltage;
        manager->rails[rail].voltage = current;

        const rail_config_t *config = &RAIL_CONFIGS[rail];

        if (!manager->rails[rail].enabled) {
            continue;
        }

        rail_status_t new_status = RAIL_STATUS_OK;
        osusat_event_id_t event_id = 0;

        if (current > config->current_limit) {
            new_status = RAIL_STATUS_OVERCURRENT;
            event_id = RAIL_CONTROLLER_EVENT_OVERCURRENT_DETECTED;

            printf("FAULT: %s overcurrent: %.2fA (limit: %.2fA)\n",
                   config->name, current, config->current_limit);
        } else if (voltage < config->voltage_min) {
            new_status = RAIL_STATUS_UNDERVOLTAGE;
            event_id = RAIL_CONTROLLER_EVENT_UNDERVOLTAGE_DETECTED;

            printf("FAULT: %s undervoltage: %.2fV (min: %.2fV)\n", config->name,
                   voltage, config->voltage_min);
        } else if (voltage > config->voltage_max) {
            new_status = RAIL_STATUS_OVERVOLTAGE;
            event_id = RAIL_CONTROLLER_EVENT_OVERVOLTAGE_DETECTED;

            printf("FAULT: %s overvoltage: %.2fV (max: %.2fV)\n", config->name,
                   voltage, config->voltage_max);
        }

        if (new_status != manager->rails[rail].status) {
            manager->rails[rail].status = new_status;

            if (new_status != RAIL_STATUS_OK) {
                osusat_event_bus_publish(event_id, &rail, sizeof(size_t));

                manager->rails[rail].enabled = false;
            }
        }
    }
}
