/**
 * @file hal_gpio_types.h
 * @brief GPIO type definitions shared between HAL and configuration
 */
#ifndef HAL_GPIO_TYPES_H
#define HAL_GPIO_TYPES_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @enum gpio_mode_t
 * @brief Possible GPIO input modes
 */
typedef enum {
    HAL_GPIO_MODE_INPUT,            /**< GPIO input mode (reading) */
    HAL_GPIO_MODE_OUTPUT,           /**< GPIO output mode (driving) */
    HAL_GPIO_MODE_IT_RISING,        /**< Interrupt on rising edge */
    HAL_GPIO_MODE_IT_FALLING,       /**< Interrupt on falling edge */
    HAL_GPIO_MODE_IT_RISING_FALLING /**< Interrupt on both edges */
} gpio_mode_t;

/**
 * @enum gpio_state_t
 * @brief Possible GPIO states
 */
typedef enum {
    HAL_GPIO_STATE_LOW,  /**< The GPIO pin is currently low */
    HAL_GPIO_STATE_HIGH, /**< The GPIO pin is currently high */
} gpio_state_t;

/**
 * @enum gpio_pull_t
 * @brief GPIO pull configuration
 */
typedef enum {
    HAL_GPIO_PULL_UP,   /**< The GPIO pin will be internally pulled up */
    HAL_GPIO_PULL_DOWN, /**< The GPIO pin will be internally pulled down */
} gpio_pull_t;

#endif // HAL_GPIO_TYPES_H
