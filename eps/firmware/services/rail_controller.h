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

#include "eps_config.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * @defgroup rail_controller Rail Controller Service
 * @brief Driver for controlling power rails and monitoring
 * consumption.
 *
 * @note The application that consumes this service should subscribe to
 * faults (like overcurrent events) to ensure they are caught quickly.
 *
 * @{
 */

/**
 * @defgroup rail_controller_types Types
 * @ingroup rail_controller
 * @brief Structures and Enums for rail status monitoring.
 *
 * @{
 */

#define RAIL_CONTROLLER_SERVICE_ID 0xAC25

typedef enum {
    /**
     * @brief Published when a critical fault is detected.
     * Payload: rail_controller_t (Snapshot at time of failure)
     */
    RAIL_CONTROLLER_FAULT_DETECTED = 0x10,

    /**
     * @brief Published when an overcurrent is detected on a single rail.
     * Payload: rail_t (Snapshot at time of failure)
     */
    RAIL_CONTROLLER_OVERCURRENT_DETECTED,

    /**
     * @brief Published when an undervoltage event is detected on a single rail.
     * Payload: rail_t (Snapshot at time of failure)
     */
    RAIL_CONTROLLER_UNDERVOLTAGE_DETECTED,

    /**
     * @brief Published when an overvoltage event is detected on a single rail.
     * Payload: rail_t (Snapshot at time of failure)
     */
    RAIL_CONTROLLER_OVERVOLTAGE_DETECTED,

    /**
     * @brief Published when a critical fault is detected on a single rail.
     * Payload: rail_t (Snapshot at time of failure)
     */
    RAIL_CONTROLLER_RAIL_FAULT_DETECTED,

    /**
     * @brief Telemetry snapshot of one rail. Published for each rail
     * Payload: rail_t (Snapshot)
     */
    RAIL_CONTROLLER_TELEMETRY,
} rail_controller_id_t;

#define RAIL_CONTROLLER_EVENT_FAULT_DETECTED                                   \
    OSUSAT_BUILD_EVENT_ID(RAIL_CONTROLLER_SERVICE_ID,                          \
                          RAIL_CONTROLLER_FAULT_DETECTED)
#define RAIL_CONTROLLER_EVENT_OVERCURRENT_DETECTED                             \
    OSUSAT_BUILD_EVENT_ID(RAIL_CONTROLLER_SERVICE_ID,                          \
                          RAIL_CONTROLLER_OVERCURRENT_DETECTED)
#define RAIL_CONTROLLER_EVENT_UNDERVOLTAGE_DETECTED                            \
    OSUSAT_BUILD_EVENT_ID(RAIL_CONTROLLER_SERVICE_ID,                          \
                          RAIL_CONTROLLER_UNDERVOLTAGE_DETECTED)
#define RAIL_CONTROLLER_EVENT_OVERVOLTAGE_DETECTED                             \
    OSUSAT_BUILD_EVENT_ID(RAIL_CONTROLLER_SERVICE_ID,                          \
                          RAIL_CONTROLLER_OVERVOLTAGE_DETECTED)
#define RAIL_CONTROLLER_EVENT_RAIL_FAULT_DETECTED                              \
    OSUSAT_BUILD_EVENT_ID(RAIL_CONTROLLER_SERVICE_ID,                          \
                          RAIL_CONTROLLER_RAIL_FAULT_DETECTED)

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
    RAIL_STATUS_OVERVOLTAGE,  /** <Rail voltage is above expected threshold */
    RAIL_STATUS_FAULT         /**< Generic fault or hardware failure */
} rail_status_t;

/**
 * @struct rail_t
 * @brief Snapshot for a specific power rail.
 *
 * Returned by ::rail_controller_get_rail for system monitoring.
 */
typedef struct {
    power_rail_t rail_id; /**< The rail this structure represents */
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
    rail_t rails[NUM_POWER_RAILS]; /**< Array of rail snapshots (length =
                                       NUM_POWER_RAILS) */
    bool initialized;      /**< True if controller has been initialized */
    uint32_t tick_counter; /**< Internal counter for update loop */
    uint32_t telemetry_tick_counter; /**< Internal counter for telemetry */
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
void rail_controller_init(rail_controller_t *manager);

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
void rail_controller_enable(rail_controller_t *manager, power_rail_t rail);

/**
 * @brief Turn OFF a specific power rail.
 *
 * @param[in] rail The unique ID of the rail to disable.
 * @param[in] controller The rail controller
 */
void rail_controller_disable(rail_controller_t *manager, power_rail_t rail);

/** @} */ // end rail_controller_api

/** @} */ // end rail_controller

#endif // RAIL_CONTROLLER_H
