#ifndef OBC_CONFIG_H
#define OBC_CONFIG_H

#include "hal_gpio_types.h"
#include <stdint.h>

#define NUM_GPIO_PINS 13

// FDCAN peripheral pins
// FDCAN1: RX -> PB8, TX -> PB9
#define FDCAN1_RX_PORT 1
#define FDCAN1_RX_PIN 8
#define FDCAN1_TX_PORT 1
#define FDCAN1_TX_PIN 9

// FDCAN2: RX -> PB5, TX -> PB6
#define FDCAN2_RX_PORT 1
#define FDCAN2_RX_PIN 5
#define FDCAN2_TX_PORT 1
#define FDCAN2_TX_PIN 6

// USART peripheral pins
// USART1: TX -> PA9, RX -> PA10
#define USART1_TX_PORT 0
#define USART1_TX_PIN 9
#define USART1_RX_PORT 0
#define USART1_RX_PIN 10

// USART6: TX -> PC6, RX -> PC7
#define USART6_TX_PORT 2
#define USART6_TX_PIN 6
#define USART6_RX_PORT 2
#define USART6_RX_PIN 7

// UART7: TX -> PE8, RX -> PE7
#define UART7_TX_PORT 4
#define UART7_TX_PIN 8
#define UART7_RX_PORT 4
#define UART7_RX_PIN 7

#define SERVICE_COUNT 4

#define WATCHDOG_WDI_PIN 8
#define WATCHDOG_SET0_PIN 9
#define WATCHDOG_SET1_PIN 10
#define WATCHDOG_SET2_PIN 11

#define GREEN_LED_PIN 0
#define RED_LED_PIN 12

typedef uint8_t gpio_port_id_t; // e.g., 0 = Port A, 1 = Port B, etc.
typedef uint8_t gpio_pin_id_t;  // e.g., 0..15

typedef struct {
    gpio_port_id_t port;      /**< Abstract port ID */
    gpio_pin_id_t pin;        /**< Abstract pin number */
    gpio_pull_t pull;         /**< Default pull */
    gpio_mode_t default_mode; /**< Initial mode */
} gpio_config_t;

static const gpio_config_t gpio_board_config[NUM_GPIO_PINS] = {
    {3, 10, HAL_GPIO_NO_PULL,
     HAL_GPIO_MODE_OUTPUT}, // index 0: PD10 (Green LED1)
    {1, 14, HAL_GPIO_NO_PULL,
     HAL_GPIO_MODE_OUTPUT}, // index 1: PB14 (e.g. LED2)
    {2, 13, HAL_GPIO_NO_PULL,
     HAL_GPIO_MODE_INPUT}, // index 2: PC13 (e.g. BUTTON1)
    {3, 3, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_OUTPUT}, // index 3: PD3
    {4, 4, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_OUTPUT}, // index 4: PE4
    {5, 5, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_OUTPUT}, // index 5: PF5
    {6, 6, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_OUTPUT}, // index 6: PG6
    {7, 7, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_OUTPUT}, // index 7: PH7
    {2, 1, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_OUTPUT}, // index 8: PC1 (WDI)
    {0, 0, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_OUTPUT}, // index 9: PA0 (WD_S0)
    {0, 1, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_OUTPUT}, // index 10: PA1 (WD_S1)
    {0, 2, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_OUTPUT}, // index 11: PA2 (WD_S2)
    {3, 11, HAL_GPIO_NO_PULL, HAL_GPIO_MODE_OUTPUT}, // index 12: PD11 (Red LED)
};

#endif // OBC_CONFIG_H
