#ifndef GPIO_H
#define GPIO_H

#include "hal_gpio_types.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * @typedef gpio_callback_t
 * @brief Function signature for GPIO interrupt handlers.
 *
 * @param[in] pin The pin ID that triggered the interrupt.
 * @param[in] ctx User context pointer provided at registration.
 */
typedef void (*gpio_callback_t)(uint8_t pin, void *ctx);

/**
 * @enum gpio_pin_t
 * @brief GPIO pin descriptor
 */
typedef struct {
    gpio_mode_t mode;   /**< GPIO pin mode */
    gpio_state_t state; /**< GPIO state mode */
    gpio_pull_t pull;   /**< GPIO pull */
    bool irq_enabled;   /**< Whether the IRQ is enabled on this pin */

    gpio_callback_t cb; /**< Callback that fires when pin state changes */
    void *ctx;          /**< Context passed to the callback when fired */
} gpio_pin_t;

/**
 * @brief Initialize the GPIO driver.
 */
void hal_gpio_init(void);

/**
 * @brief Set the mode of a GPIO pin.
 */
void hal_gpio_set_mode(uint8_t pin, gpio_mode_t mode);

/**
 * @brief Register a software callback for a GPIO interrupt.
 */
void hal_gpio_register_callback(uint8_t pin, gpio_callback_t callback,
                                void *ctx);

/**
 * @brief Drive a GPIO pin
 */
void hal_gpio_write(uint8_t pin, gpio_state_t state);

/**
 * @brief Read the state of a GPIO pin
 */
gpio_state_t hal_gpio_read(uint8_t pin);

/**
 * @brief Toggle a GPIO pin from its current state
 */
void hal_gpio_toggle(uint8_t pin);

#endif // GPIO_H
