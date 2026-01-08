#ifndef EPS_CONFIG_H
#define EPS_CONFIG_H

#include "hal_gpio_types.h"
#include <stdint.h>

#define NUM_MPPT_CHANNELS 1 // the number of MPPT channels per device
#define NUM_POWER_RAILS 8   // the number of power rails available on the EPS
#define NUM_GPIO_PINS 36    // number of GPIO pins in use

#define CRITICAL_BATTERY_VOLTAGE_THRESHOLD 3.3f

#define SERVICE_COUNT 8

#define I2C_TIMING_BITFIELD 0x10D19CE4

typedef enum { // these rails correspond to the hardware rails
    RAIL_OBC,
    RAIL_RADIO,
    RAIL_GPS,
    RAIL_PAYLOAD_1,
    RAIL_PAYLOAD_2,
    RAIL_5V_BUS,
    RAIL_3V3_BUS,
    // TODO: add more rails as needed
} power_rail_t;

typedef uint8_t gpio_port_id_t; // e.g., 0 = Port A, 1 = Port B, 2 = Port C
typedef uint8_t gpio_pin_id_t;  // e.g., 0..15

typedef struct {
    gpio_port_id_t port;      /**< Abstract port ID */
    gpio_pin_id_t pin;        /**< Abstract pin number */
    gpio_pull_t pull;         /**< Default pull */
    gpio_mode_t default_mode; /**< Initial mode */
} gpio_config_t;

typedef struct {
    power_rail_t rail_id;
    float nominal_voltage; /**< Expected voltage (e.g., 5.0, 3.3) */
    float voltage_min;     /**< Minimum acceptable voltage */
    float voltage_max;     /**< Maximum acceptable voltage */
    float current_limit;   /**< Maximum current before fault */
    const char *name;      /**< Human-readable name for logging */
} rail_config_t;

static const gpio_config_t gpio_board_config[NUM_GPIO_PINS] = {
    // power rail state tracking (note that the pin is pulled up by hardware)
    {0, 0, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_IT_RISING_FALLING},
    {0, 1, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_IT_RISING_FALLING},
    {0, 2, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_IT_RISING_FALLING},
    {0, 3, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_IT_RISING_FALLING},
    {0, 4, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_IT_RISING_FALLING},
    {0, 5, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_IT_RISING_FALLING},
    {0, 6, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_IT_RISING_FALLING},
    {0, 7, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_IT_RISING_FALLING},
    // power rail control
    {3, 0, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_OUTPUT},
    {3, 1, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_OUTPUT},
    {3, 2, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_OUTPUT},
    {3, 3, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_OUTPUT},
    {3, 4, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_OUTPUT},
    {3, 5, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_OUTPUT},
    {3, 6, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_OUTPUT},
    {3, 7, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_OUTPUT},
    {3, 8, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_OUTPUT},
    {3, 9, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_OUTPUT},
    {3, 10, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_OUTPUT},
    {3, 11, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_OUTPUT},
    {3, 12, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_OUTPUT},
    {3, 13, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_OUTPUT},
    {3, 14, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_OUTPUT},
    {3, 15, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_OUTPUT},
};

static const rail_config_t RAIL_CONFIGS[NUM_POWER_RAILS] = {
    [RAIL_OBC] = {.rail_id = RAIL_OBC,
                  .nominal_voltage = 3.3f,
                  .voltage_min = 3.0f,
                  .voltage_max = 3.6f,
                  .current_limit = 2.0f,
                  .name = "OBC"},
    [RAIL_RADIO] = {.rail_id = RAIL_RADIO,
                    .nominal_voltage = 5.0f,
                    .voltage_min = 4.75f,
                    .voltage_max = 5.25f,
                    .current_limit = 1.5f,
                    .name = "Radio"},
    [RAIL_GPS] = {.rail_id = RAIL_GPS,
                  .nominal_voltage = 3.3f,
                  .voltage_min = 3.0f,
                  .voltage_max = 3.6f,
                  .current_limit = 0.5f,
                  .name = "GPS"},
    [RAIL_PAYLOAD_1] = {.rail_id = RAIL_PAYLOAD_1,
                        .nominal_voltage = 5.0f,
                        .voltage_min = 4.75f,
                        .voltage_max = 5.25f,
                        .current_limit = 3.0f,
                        .name = "Payload 1"},
    [RAIL_PAYLOAD_2] = {.rail_id = RAIL_PAYLOAD_2,
                        .nominal_voltage = 5.0f,
                        .voltage_min = 4.75f,
                        .voltage_max = 5.25f,
                        .current_limit = 3.0f,
                        .name = "Payload 2"},
    [RAIL_5V_BUS] = {.rail_id = RAIL_5V_BUS,
                     .nominal_voltage = 5.0f,
                     .voltage_min = 4.75f,
                     .voltage_max = 5.25f,
                     .current_limit = 5.0f,
                     .name = "5V Bus"},
    [RAIL_3V3_BUS] = {.rail_id = RAIL_3V3_BUS,
                      .nominal_voltage = 3.3f,
                      .voltage_min = 3.0f,
                      .voltage_max = 3.6f,
                      .current_limit = 4.0f,
                      .name = "3.3V Bus"}};

#endif
