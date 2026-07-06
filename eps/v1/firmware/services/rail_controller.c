#include "rail_controller.h"
#include "eps_config.h"
#include "events.h"
#include "logging.h"
#include "osusat/event_bus.h"
#include "hal_gpio.h"
#include "hal_adc.h"
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

static void rail_monitor_callback(uint8_t pin, void *ctx) {
    rail_controller_t *manager = (rail_controller_t *)ctx;
    if (pin >= NUM_POWER_RAILS || manager == NULL) {
        return;
    }

    gpio_state_t state = hal_gpio_read(pin);
    LOG_INFO(EPS_COMPONENT_RAIL, "RAIL EXTI: Rail %d (monitoring pin PB%d) transitioned to %s",
             (int)pin, (int)(8 + pin), (state == HAL_GPIO_STATE_HIGH) ? "HIGH" : "LOW");
    printf("RAIL EXTI: Rail %d (monitoring pin PB%d) transitioned to %s\n",
           (int)pin, (int)(8 + pin), (state == HAL_GPIO_STATE_HIGH) ? "HIGH" : "LOW");
}

void rail_controller_init(rail_controller_t *manager) {
    if (manager == NULL) {
        return;
    }

    memset(manager, 0, sizeof(rail_controller_t));

    for (size_t rail = 0; rail < NUM_POWER_RAILS; rail++) {
        manager->rails[rail].rail_id = (power_rail_t)rail;
        manager->rails[rail].status = RAIL_STATUS_DISABLED;
        manager->rails[rail].enabled = false;

        // Register EXTI monitoring callbacks for the rails (pins 0 to 7 correspond to PB8-PB15)
        if (rail < 8) {
            hal_gpio_register_callback(rail, rail_monitor_callback, manager);
        }
    }

    // TODO: bringup?

    manager->initialized = true;
    LOG_INFO(EPS_COMPONENT_RAIL, "Rail controller initialized");
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

    LOG_INFO(EPS_COMPONENT_RAIL, "Enabling rail %d", rail);
    hal_gpio_write(8 + rail, HAL_GPIO_STATE_HIGH);

    controller->rails[rail].enabled = true;
    controller->rails[rail].status = RAIL_STATUS_OK;
}

void rail_controller_disable(rail_controller_t *controller, power_rail_t rail) {
    if (controller == NULL || rail >= NUM_POWER_RAILS) {
        return;
    }

    LOG_INFO(EPS_COMPONENT_RAIL, "Disabling rail %d", rail);
    hal_gpio_write(8 + rail, HAL_GPIO_STATE_LOW);

    controller->rails[rail].enabled = false;
    controller->rails[rail].status = RAIL_STATUS_DISABLED;
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
        float voltage = 0.0f;
        float current = 0.0f;

        if (manager->rails[rail].enabled) {
            voltage = RAIL_CONFIGS[rail].nominal_voltage;
            // Read raw current from corresponding ADC channel (0-7)
            uint16_t raw_adc = hal_adc_read((adc_channel_t)rail);
            // Map 12-bit ADC full scale (4095) to 120% of current limit
            current = (raw_adc / 4095.0f) * RAIL_CONFIGS[rail].current_limit * 1.2f;
        }

        manager->rails[rail].voltage = voltage;
        manager->rails[rail].current = current;

        const rail_config_t *config = &RAIL_CONFIGS[rail];

        if (!manager->rails[rail].enabled) {
            continue;
        }

        rail_status_t new_status = RAIL_STATUS_OK;
        osusat_event_id_t event_id = 0;

        if (current > config->current_limit) {
            new_status = RAIL_STATUS_OVERCURRENT;
            event_id = RAIL_CONTROLLER_EVENT_OVERCURRENT_DETECTED;

            LOG_ERROR(EPS_COMPONENT_RAIL,
                      "%s overcurrent: %.2fA (limit: %.2fA)", config->name,
                      current, config->current_limit);
            printf("FAULT: %s overcurrent: %.2fA (limit: %.2fA)\n",
                   config->name, current, config->current_limit);
        } else if (voltage < config->voltage_min) {
            new_status = RAIL_STATUS_UNDERVOLTAGE;
            event_id = RAIL_CONTROLLER_EVENT_UNDERVOLTAGE_DETECTED;

            LOG_ERROR(EPS_COMPONENT_RAIL, "%s undervoltage: %.2fV (min: %.2fV)",
                      config->name, voltage, config->voltage_min);
            printf("FAULT: %s undervoltage: %.2fV (min: %.2fV)\n", config->name,
                   voltage, config->voltage_min);
        } else if (voltage > config->voltage_max) {
            new_status = RAIL_STATUS_OVERVOLTAGE;
            event_id = RAIL_CONTROLLER_EVENT_OVERVOLTAGE_DETECTED;

            LOG_ERROR(EPS_COMPONENT_RAIL, "%s overvoltage: %.2fV (max: %.2fV)",
                      config->name, voltage, config->voltage_max);
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
