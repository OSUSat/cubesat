#ifndef EPS_CONFIG_H
#define EPS_CONFIG_H

#include "hal_gpio_types.h"
#include <stdint.h>

#define NUM_MPPT_CHANNELS 1 // the number of MPPT channels per device
#define NUM_POWER_RAILS 8   // the number of power rails available on the EPS
#define NUM_GPIO_PINS 31    // number of GPIO pins in use

// i2c peripheral pins
// i2c1: scl -> pg14, sda -> pg13
#define I2C1_SCL_PORT 6
#define I2C1_SCL_PIN 14
#define I2C1_SDA_PORT 6
#define I2C1_SDA_PIN 13

// i2c2: scl -> pb10, sda -> pb11
#define I2C2_SCL_PORT 1
#define I2C2_SCL_PIN 10
#define I2C2_SDA_PORT 1
#define I2C2_SDA_PIN 11

// i2c3: scl -> pg7, sda -> pg8
#define I2C3_SCL_PORT 6
#define I2C3_SCL_PIN 7
#define I2C3_SDA_PORT 6
#define I2C3_SDA_PIN 8

// i2c4: scl -> pf14, sda -> pf15
#define I2C4_SCL_PORT 5
#define I2C4_SCL_PIN 14
#define I2C4_SDA_PORT 5
#define I2C4_SDA_PIN 15

// usart peripheral pins
// usart1: tx -> pg9, rx -> pg10
#define USART1_TX_PORT 6
#define USART1_TX_PIN 9
#define USART1_RX_PORT 6
#define USART1_RX_PIN 10

// usart3: tx -> pc4, rx -> pc5
#define USART3_TX_PORT 2
#define USART3_TX_PIN 4
#define USART3_RX_PORT 2
#define USART3_RX_PIN 5

// can peripheral pins
// can1: rx -> pa11, tx -> pa12
#define CAN1_RX_PORT 0
#define CAN1_RX_PIN 11
#define CAN1_TX_PORT 0
#define CAN1_TX_PIN 12

// can2: rx -> pb5, tx -> pb6
#define CAN2_RX_PORT 1
#define CAN2_RX_PIN 5
#define CAN2_TX_PORT 1
#define CAN2_TX_PIN 6

// adc2 channel input pins
// pc2 -> adc2_in3 (adcx_in3)
#define ADC2_IN3_PORT 2
#define ADC2_IN3_PIN 2

// pc3 -> adc2_in4 (adcx_in4)
#define ADC2_IN4_PORT 2
#define ADC2_IN4_PIN 3

// pa0 -> adc2_in5 (adcx_in5)
#define ADC2_IN5_PORT 0
#define ADC2_IN5_PIN 0

// pa1 -> adc2_in6 (adcx_in6)
#define ADC2_IN6_PORT 0
#define ADC2_IN6_PIN 1

// pa2 -> adc2_in7 (adcx_in7)
#define ADC2_IN7_PORT 0
#define ADC2_IN7_PIN 2

// pa3 -> adc2_in8 (adcx_in8)
#define ADC2_IN8_PORT 0
#define ADC2_IN8_PIN 3

// pa4 -> adc2_in9 (adcx_in9)
#define ADC2_IN9_PORT 0
#define ADC2_IN9_PIN 4

// pa5 -> adc2_in10 (adcx_in10)
#define ADC2_IN10_PORT 0
#define ADC2_IN10_PIN 5

// pa6 -> adc2_in11 (adcx_in11)
#define ADC2_IN11_PORT 0
#define ADC2_IN11_PIN 6

// pa7 -> adc2_in12 (adcx_in12)
#define ADC2_IN12_PORT 0
#define ADC2_IN12_PIN 7

// pb0 -> adc2_in15 (adcx_in15)
#define ADC2_IN15_PORT 1
#define ADC2_IN15_PIN 0

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
    RAIL_AUX, // 8th rail
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
    // power rail state tracking (monitoring)
    {1, 8, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_IT_RISING_FALLING},  // index 0 (PB8)
    {1, 9, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_IT_RISING_FALLING},  // index 1 (PB9)
    {1, 10, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_IT_RISING_FALLING}, // index 2 (PB10)
    {1, 11, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_IT_RISING_FALLING}, // index 3 (PB11)
    {1, 12, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_IT_RISING_FALLING}, // index 4 (PB12)
    {1, 13, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_IT_RISING_FALLING}, // index 5 (PB13)
    {1, 14, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_IT_RISING_FALLING}, // index 6 (PB14)
    {1, 15, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_IT_RISING_FALLING}, // index 7 (PB15)

    // power rail control (ON pins)
    {3, 0, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_OUTPUT},             // index 8 (PD0)
    {3, 2, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_OUTPUT},             // index 9 (PD2)
    {3, 4, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_OUTPUT},             // index 10 (PD4)
    {3, 6, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_OUTPUT},             // index 11 (PD6)
    {3, 8, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_OUTPUT},             // index 12 (PD8)
    {3, 10, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_OUTPUT},            // index 13 (PD10)
    {3, 12, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_OUTPUT},            // index 14 (PD12)
    {3, 14, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_OUTPUT},            // index 15 (PD14)

    // MAX6369 external hardware watchdog WDI pin (Port E Pin 5)
    {4, 5, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_OUTPUT},             // index 16 (PE5)
    // Watchdog SET0 pin (Port E Pin 3)
    {4, 3, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_OUTPUT},             // index 17 (PE3)
    // Watchdog SET1 pin (Port E Pin 4)
    {4, 4, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_OUTPUT},             // index 18 (PE4)

    // CANBus Transciever 1 Pins
    {0, 11, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_INPUT},             // index 19 (PA11 - RX)
    {0, 12, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_OUTPUT},            // index 20 (PA12 - TX)
    // CANBus Transciever 2 Pins
    {1, 5, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_INPUT},              // index 21 (PB5 - RX)
    {1, 6, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_OUTPUT},             // index 22 (PB6 - TX)

    // power rail reset pins (PD1, PD3, PD5, PD7, PD9, PD11, PD13, PD15)
    {3, 1, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_OUTPUT},             // index 23 (PD1)
    {3, 3, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_OUTPUT},             // index 24 (PD3)
    {3, 5, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_OUTPUT},             // index 25 (PD5)
    {3, 7, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_OUTPUT},             // index 26 (PD7)
    {3, 9, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_OUTPUT},             // index 27 (PD9)
    {3, 11, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_OUTPUT},            // index 28 (PD11)
    {3, 13, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_OUTPUT},            // index 29 (PD13)
    {3, 15, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_OUTPUT},            // index 30 (PD15)
};

#define WATCHDOG_WDI_PIN 16
#define WATCHDOG_SET0_PIN 17
#define WATCHDOG_SET1_PIN 18
#define RAIL_RESET_PIN_START 23

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
                      .name = "3.3V Bus"},
    [RAIL_AUX] = {.rail_id = RAIL_AUX,
                  .nominal_voltage = 3.3f,
                  .voltage_min = 3.0f,
                  .voltage_max = 3.6f,
                  .current_limit = 1.0f,
                  .name = "Aux Rail"}};

#endif
