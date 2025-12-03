/**
 * @file gpio.h
 * @brief GPIO hardware abstraction public API.
 *
 * This driver provides a simple interface for managing GPIO pins. It is used by
 * services to manage hardware peripherals.
 */

#ifndef GPIO_H
#define GPIO_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @defgroup gpio GPIO
 * @brief Handles GPIO control.
 * @{
 */

/**
 * @defgroup gpio_types Types
 * @ingroup gpio
 * @brief Types used by the GPIO driver.
 *
 * @{
 */

/**
 * @enum gpio_mode_t
 * @brief Possible GPIO input modes
 */
typedef enum {
    GPIO_MODE_INPUT,            /**< GPIO input mode (reading) */
    GPIO_MODE_OUTPUT,           /**< GPIO output mode (driving) */
    GPIO_MODE_IT_RISING,        /**< Interrupt on rising edge */
    GPIO_MODE_IT_FALLING,       /**< Interrupt on falling edge */
    GPIO_MODE_IT_RISING_FALLING /**< Interrupt on both edges */
} gpio_mode_t;

/**
 * @enum gpio_state_t
 * @brief Possible GPIO states
 */
typedef enum {
    GPIO_STATE_LOW,  /**< The GPIO pin is currently low */
    GPIO_STATE_HIGH, /**< The GPIO pin is currently high */
} gpio_state_t;

/**
 * @typedef gpio_callback_t
 * @brief Function signature for GPIO interrupt handlers.
 *
 * @param[in] pin The pin ID that triggered the interrupt.
 * @param[in] ctx User context pointer provided at registration.
 */
typedef void (*gpio_callback_t)(uint8_t pin, void *ctx);

/** @} */ // end gpio_types

/**
 * @defgroup gpio_api Public API
 * @ingroup gpio
 * @brief External interface for interacting with the GPIO drvier.
 *
 * @{
 */

/**
 * @brief Initialize the GPIO driver.
 *
 * This should be called before using any other GPIO functions.
 */
void gpio_init(void);

/**
 * @brief Set the mode of a GPIO pin.
 *
 * If an interrupt mode is selected, this function will configure
 * the NVIC (Interrupt Controller) but will NOT enable the callback
 * until gpio_register_callback() is called.
 *
 * @param[in] pin The GPIO pin
 * @param[in] mode The GPIO mode
 */
void gpio_set_mode(uint8_t pin, gpio_mode_t mode);

/**
 * @brief Register a software callback for a GPIO interrupt.
 *
 * When the hardware interrupt fires, the driver will look up this
 * function and execute it.
 *
 * @param[in] pin      The GPIO pin ID.
 * @param[in] callback The function to call when interrupt triggers.
 * @param[in] ctx      Optional context pointer (e.g., Service struct or Event
 * Bus).
 */
void gpio_register_callback(uint8_t pin, gpio_callback_t callback, void *ctx);

/**
 * @brief Drive a GPIO pin
 *
 * @param[in] pin The pin to drive
 * @param[in] state The state to drive the pin to
 */
void gpio_write(uint8_t pin, gpio_state_t state);

/**
 * @brief Read the state of a GPIO pin
 *
 * @param[in] pin The pin to read
 *
 * @retval GPIO_STATE_LOW the GPIO pin is currently low
 * @retval GPIO_STATE_HIGH the GPIO pin is currently high
 */
gpio_state_t gpio_read(uint8_t pin);

/**
 * @brief Toggle a GPIO pin from its current state
 *
 * @param[in] pin The pin to toggle
 */
void gpio_toggle(uint8_t pin);

/** @} */ // end gpio_api

/** @} */ // end gpio

#endif // GPIO_H
