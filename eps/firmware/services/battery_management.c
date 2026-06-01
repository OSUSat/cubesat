/**
 * @file battery_management.c
 * @brief Battery Management Service Implementation
 */

#include "battery_management.h"
#include "eps_config.h"
#include "hal_adc.h"
#include "hal_gpio.h"
#include "hal_i2c.h"
#include "osusat/event_bus.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define BATTERY_CHARGE_PIN 24
#define BATTERY_BALANCE_PIN 25

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

    hal_gpio_set_mode(BATTERY_CHARGE_PIN, HAL_GPIO_MODE_OUTPUT);
    hal_gpio_set_mode(BATTERY_BALANCE_PIN, HAL_GPIO_MODE_OUTPUT);
    hal_gpio_write(BATTERY_CHARGE_PIN, HAL_GPIO_STATE_LOW);
    hal_gpio_write(BATTERY_BALANCE_PIN, HAL_GPIO_STATE_LOW);

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
    manager->battery_status.balancing = enable;

    gpio_state_t state = enable ? HAL_GPIO_STATE_HIGH : HAL_GPIO_STATE_LOW;
    hal_gpio_write(BATTERY_CHARGE_PIN, state);
    hal_gpio_write(BATTERY_BALANCE_PIN, state);

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

    manager->tick_counter++;

    if (manager->tick_counter >= BATTERY_UPDATE_INTERVAL_TICKS) {
        manager->tick_counter = 0;
        battery_perform_update(manager);
    }
}

static void battery_perform_update(battery_management_t *manager) {
    if (manager == NULL) {
        return;
    }

    uint16_t raw_voltage = hal_adc_read(ADC_CHANNEL_0);
    float voltage = ((float)raw_voltage / 4095.0f) * 4.5f;

    manager->battery_status.voltage = voltage;

    if (voltage < CRITICAL_BATTERY_VOLTAGE_THRESHOLD &&
        !manager->battery_status.protection) {
        battery_protect_mode(manager);

        osusat_event_bus_publish(BATTERY_EVENT_CRITICAL_LOW, &voltage,
                                 sizeof(float));
    }

    manager->telemetry_tick_counter++;

    if (manager->telemetry_tick_counter >= TELEMETRY_INTERVAL_CYCLES) {
        manager->telemetry_tick_counter = 0;

        osusat_event_bus_publish(BATTERY_EVENT_TELEMETRY,
                                 &manager->battery_status,
                                 sizeof(battery_status_t));
    }
}

static bool battery_run_diagnostics(battery_management_t *manager) {
    if (manager == NULL) {
        return false;
    }

    uint8_t dummy_buf = 0;
    i2c_error_t i2c_err = hal_i2c_mem_read(I2C_BUS_1, 0x6A, 0x26, &dummy_buf, 1,
                                           NULL, NULL, NULL);
    if (i2c_err != I2C_HAL_ERR_NONE) {
        return false;
    }

    uint16_t raw_voltage = hal_adc_read(ADC_CHANNEL_0);
    float voltage = ((float)raw_voltage / 4095.0f) * 4.5f;

    if (voltage > 0.0f && voltage < CRITICAL_BATTERY_VOLTAGE_THRESHOLD) {
        return false;
    }

    return true;
}
