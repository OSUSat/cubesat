/**
 * @file watchdog.h
 * @brief Hardware Watchdog Timer (WDT) abstraction.
 *
 * This module manages the system's Independent Watchdog.
 */

#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @defgroup watchdog Watchdog Service
 * @brief Safety mechanism to recover from software freezes.
 *
 * The Watchdog service is a high-priority driver that interfaces directly
 * with the MCU's hardware timer.
 *
 * @{
 */

/**
 * @defgroup watchdog_types Types
 * @ingroup watchdog
 *
 * @brief State containers for the watchdog.
 *
 * @{
 */

/**
 * @struct watchdog_t
 * @brief Watchdog Service State Object.
 *
 * Holds the runtime state of the watchdog driver.
 */
typedef struct {
    uint32_t timeout_ms; /**< The configured timeout period in milliseconds. */
    uint32_t last_pet_tick; /**< Timestamp of the last kick. */
    bool enabled;           /**< Hardware enablement status. */
} watchdog_t;

/** @} */ // end watchdog_types

/**
 * @defgroup watchdog_api Public API
 * @ingroup watchdog
 *
 * @brief Functions for managing system safety.
 *
 * @{
 */

/**
 * @brief Initialize the Hardware Watchdog.
 *
 * Configures the hardware timer with the timeout specified in the
 * configuration. Once initialized, the watchdog typically cannot be disabled
 * until a reset.
 *
 * @param[out] wd Pointer to the watchdog service instance.
 */
void watchdog_init(watchdog_t *watchdog);

/**
 * @brief Reset the watchdog timer (kicking/petting).
 *
 * Refreshes the hardware counter to prevent a system reset.
 * Updates the `last_pet_tick` in the software struct for debugging.
 *
 * @param[in,out] watchdog Pointer to the watchdog service instance.
 */
void watchdog_pet(watchdog_t *watchdog);

/**
 * @brief Intentionally trigger a system reset.
 *
 * Useful for "safe mode" transitions or recovering from unrecoverable errors.
 * This function will block until the hardware resets the device.
 *
 * @param[in] watchdog Pointer to the watchdog service instance.
 */
void watchdog_force_reset(watchdog_t *watchdog);

/** @} */ // end watchdog_api

/** @} */ // end watchdog

#endif // WATCHDOG_H
