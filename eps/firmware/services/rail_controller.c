#include "rail_controller.h"
#include "eps_config.h"
#include "events.h"
#include "hal_gpio.h"
#include "osusat/event_bus.h"
#include <stdio.h>
#include <string.h>

#define RAIL_CONTROL_GPIO_PIN_START 8

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

    // configure gpio pins for load switches as outputs (defaulting to off)
    for (size_t rail = 0; rail < NUM_POWER_RAILS; rail++) {
        hal_gpio_set_mode(RAIL_CONTROL_GPIO_PIN_START + rail,
                          HAL_GPIO_MODE_OUTPUT);
        hal_gpio_write(RAIL_CONTROL_GPIO_PIN_START + rail, HAL_GPIO_STATE_LOW);
    }

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
    if (controller == NULL || rail >= NUM_POWER_RAILS) {
        return;
    }

    controller->rails[rail].enabled = true;
    controller->rails[rail].status = RAIL_STATUS_OK;

    // drive the load switch gpio pin high/on
    hal_gpio_write(RAIL_CONTROL_GPIO_PIN_START + rail, HAL_GPIO_STATE_HIGH);

    printf("INFO: rail_controller_enable for rail %d\n", rail);
}

void rail_controller_disable(rail_controller_t *controller, power_rail_t rail) {
    if (controller == NULL || rail >= NUM_POWER_RAILS) {
        return;
    }

    controller->rails[rail].enabled = false;
    controller->rails[rail].status = RAIL_STATUS_DISABLED;

    // drive the load switch gpio pin low/off
    hal_gpio_write(RAIL_CONTROL_GPIO_PIN_START + rail, HAL_GPIO_STATE_LOW);

    printf("INFO: rail_controller_disable for rail %d\n", rail);
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
    if (manager == NULL) {
        return;
    }

    for (size_t rail = 0; rail < NUM_POWER_RAILS; rail++) {
        const rail_config_t *config = &RAIL_CONFIGS[rail];

        // check if this is an unconfigured rail
        if (config->name == NULL) {
            continue;
        }

        // sensible default stubs
        float voltage = manager->rails[rail].voltage;
        float current = manager->rails[rail].current;

        if (!manager->rails[rail].enabled) {
            voltage = 0.0f;
            current = 0.0f;
        } else if (voltage == 0.0f) {
            voltage = config->nominal_voltage;
            current = 0.1f;
        }

        // correct swapped assignment bug
        manager->rails[rail].voltage = voltage;
        manager->rails[rail].current = current;

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

                // disable physical switch on fault
                hal_gpio_write(RAIL_CONTROL_GPIO_PIN_START + rail,
                               HAL_GPIO_STATE_LOW);
            }
        }
    }
}
