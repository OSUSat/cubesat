/**
 * @file rail_controller.h
 * @brief Service for controlling EPS load switches and power rails.
 *
 * This module provides control over the satellite's
 * power distribution. It interacts with the GPIOs, Load Switches, and
 * Current Monitors into logical rails.
 *
 * It is responsible for:
 * - toggling physical power switches
 * - reading voltage and current telemetry
 * - implementing software-based overcurrent protection
 */

#ifndef RAIL_CONTROLLER_H
#define RAIL_CONTROLLER_H

#include "../config/eps_config.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * @defgroup rail_controller Rail Controller Service
 * @brief Driver for controlling power rails and monitoring
 * consumption.
 *
 * @note This service should be polled periodically via ::rail_controller_update
 * to ensure faults (like overcurrent events) are caught quickly.
 *
 * @{
 */

/**
 * @defgroup rail_controller_types Types
 * @ingroup rail_controller
 * @brief Structures and Enums for rail status monitoring.
 * @{
 */

/**
 * @enum rail_status_t
 * @brief Operational health status of a power rail.
 */
typedef enum {
    RAIL_STATUS_OK,       /**<Rail is operating nominally. */
    RAIL_STATUS_DISABLED, /**< Rail is intentionally disabled. */
    // TODO: ensure that firmware plays nice with hardware interruptions.
    // the hardware can shut off load switches independently of the firmware,
    // so the firmware should be able to detect this between states and
    // update accordingly.
    RAIL_STATUS_OVERCURRENT,  /**< Rail was shut down due to current limit
                                 violation. */
    RAIL_STATUS_UNDERVOLTAGE, /**< Rail voltage is below expected threshold */
    RAIL_STATUS_FAULT         /**< Generic fault or hardware failure */
} rail_status_t;

/**
 * @struct rail_t
 * @brief Snapshot for a specific power rail.
 *
 * Returned by ::rail_controller_get_rail for system monitoring.
 */
typedef struct {
    float voltage;        /**< Output voltage in volts */
    float current;        /**< Current in amps */
    rail_status_t status; /**< Current rail status */
    bool enabled;         /**< True if the switch is currently turned on */
} rail_t;

/**
 * @struct rail_controller_t
 * @brief Container for all power rail state.
 *
 * Holds snapshots for all rails and initialization status.
 * Must be initialized with ::rail_controller_init before use.
 */
typedef struct {
    rail_t *rails;    /**< Array of rail snapshots (length = NUM_POWER_RAILS) */
    bool initialized; /**< True if controller has been initialized */
} rail_controller_t;

/** @} */ // end rail_controller_types

/**
 * @defgroup rail_controller_api Public API
 * @ingroup rail_controller
 * @brief External interface for interacting with the Rail Controller Service.
 *
 * @{
 */

/**
 * @brief Initialize the Rail Controller hardware.
 *
 * Configures GPIO pins for load switches as outputs (defaulting to OFF)
 * and initializes any I2C/ADC interfaces required for current monitoring.
 *
 * @param[controller] The rail controller
 *
 * @note This function is idempotent.
 */
void rail_controller_init(rail_controller_t *controller);

/**
 * @brief Turn ON a specific power rail.
 *
 * @param[in] rail The unique ID of the rail to enable (defined in
 * eps_config.h).
 * @param[in] controller The rail controller
 *
 * @note This function returns immediately. The rail voltage may take some
 * time to stabilize.
 */
void rail_controller_enable(rail_controller_t *controller, power_rail_t rail);

/**
 * @brief Turn OFF a specific power rail.
 *
 * @param[in] rail The unique ID of the rail to disable.
 * @param[in] controller The rail controller
 */
void rail_controller_disable(rail_controller_t *controller, power_rail_t rail);

/**
 * @brief Periodic update task.
 *
 * Should be called in the main system loop.
 *
 * This function:
 *  1. Reads the latest voltage/current data from sensors.
 *  2. Checks against safety thresholds.
 *  3. If an overcurrent is detected, it will automatically call
 *
 * ::rail_controller_disable for the affected rail and set the
 * status to #RAIL_STATUS_OVERCURRENT.
 *
 * Additionally, this function is used to update the current snapshot for all
 * rails.
 *
 * @param[in] controller The rail controller
 */
void rail_controller_update(rail_controller_t *controller);

/**
 * @brief Retrieve the latest telemetry for a specific rail.
 *
 * @param[in] rail The rail ID to query.
 * @param[in] controller The rail controller
 *
 * @return ::rail_t A copy of the latest rail status structure.
 */
rail_t rail_controller_get_rail(rail_controller_t *controller,
                                power_rail_t rail);

/** @} */ // end rail_controller_api

/** @} */ // end rail_controller

#endif // RAIL_CONTROLLER_H
