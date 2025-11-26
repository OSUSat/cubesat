/**
 * @file redundancy_manager.h
 * @brief Centralized fault management and system health aggregation.
 *
 * The Redundancy Manager acts as the single source of truth for the
 * health of the EPS. It accepts error reports from low-level services
 * (Battery, MPPT, Rail Controller), aggregates them, and determines
 * the overall ::system_health_t.
 *
 * The application layer monitors this service to decide
 * if the satellite needs to enter Safe Mode or handle a fault
 * in some way.
 */

#ifndef REDUNDANCY_MANAGER_H
#define REDUNDANCY_MANAGER_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @defgroup redundancy_manager Redundancy Manager
 * @brief Aggregates component faults to determine system-wide health.
 *
 * This module simplifies complex failure scenarios. Instead of the
 * main state machine checking many different variables to see if the
 * system is safe, it only needs to check
 * ::redundancy_manager_get_system_health.
 *
 * @{
 */

/**
 * @defgroup redundancy_manager_types Types
 * @ingroup redundancy_manager
 * @brief Enumerations for fault sources and system states.
 *
 * @{
 */

/**
 * @enum fault_source_t
 * @brief Identifies the subsystem reporting a failure.
 */
typedef enum {
    FAULT_SOURCE_BATTERY, /**< BMS issues (Over/Under voltage, Temp, etc.) */
    FAULT_SOURCE_MPPT,    /**< Solar charging failures */
    FAULT_SOURCE_RAIL,    /**< Rail Controller events (Overcurrents) */
    FAULT_SOURCE_SENSOR,  /**< I2C/SPI sensor timeouts or bad data */
    // TODO: implement more as needed
} fault_source_t;

/**
 * @typedef fault_code_t
 * @brief Generic container for specific error codes.
 *
 * Specific codes are defined in the header files of the reporting modules.
 * For example, a ::FAULT_SOURCE_RAIL might report a code corresponding
 * to `RAIL_STATUS_OVERCURRENT`.
 */
typedef uint32_t fault_code_t;

/**
 * @enum system_health_t
 * @brief High-level classification of EPS health.
 *
 * Used by the Power Policy application to drive state transitions.
 */
typedef enum {
    SYSTEM_HEALTH_OK,       /**< All systems nominal. */
    SYSTEM_HEALTH_DEGRADED, /**< Non-critical faults (e.g., one sensor failed),
                               mission continues. */
    SYSTEM_HEALTH_FAULT /**< Critical failure (e.g., Battery critical), requires
                           Safe Mode. */
} system_health_t;

/**
 * @struct redundancy_manager_t
 * @brief The redundancy manager state
 */
typedef struct {
    system_health_t health; /** The current system health snapshot */
    bool initialized; /**< True if the redundancy manager is initialized. */
} redundancy_manager_t;

/** @} */ // end redundancy_manager_types

/**
 * @defgroup redundancy_manager_api Public API
 * @ingroup redundancy_manager
 * @brief Functions for reporting faults and querying health.
 *
 * @{
 */

/**
 * @brief Initialize the Redundancy Manager.
 *
 * @param[in] manager The redundancy manager to initialize
 *
 * Clears all fault registries and sets system health to ::SYSTEM_HEALTH_OK.
 */
void redundancy_manager_init(redundancy_manager_t *manager);

/**
 * @brief Report a fault from a subsystem.
 *
 * Called by other services when they detect an issue.
 * The Redundancy Manager will log this fault and potentially downgrade
 * the system health.
 *
 * @param[in] manager The redundancy manager
 * @param[in] source The subsystem reporting the error.
 * @param[in] code The specific error code (module specific).
 */
void redundancy_manager_report_fault(redundancy_manager_t *manager,
                                     fault_source_t source, fault_code_t code);

/**
 * @brief Clear a previously reported fault.
 *
 * Used when a subsystem recovers or when a fault is manually acknowledged.
 * If all critical faults are cleared, system health may return to
 * ::SYSTEM_HEALTH_OK.
 *
 * @param[in] manager The redundancy manager
 * @param[in] source The subsystem clearing the error.
 * @param[in] code The specific error code to clear.
 */
void redundancy_manager_clear_fault(redundancy_manager_t *manager,
                                    fault_source_t source, fault_code_t code);

/**
 * @brief Get the current aggregated system health.
 *
 * This is the primary interface for the application layer.
 *
 * @param[in] manager The redundancy manager
 *
 * @return ::system_health_t The worst-case health status currently active.
 */
system_health_t
redundancy_manager_get_system_health(redundancy_manager_t *manager);

/**
 * @brief Periodic update task.
 *
 * Processes counters and timeouts.
 * Use this to implement logic such as "Only trigger a system fault if
 * the sensor error persists for 5 consecutive updates."
 */
void redundancy_manager_update(redundancy_manager_t *manager);

/** @} */ // end redundancy_manager_api

/** @} */ // end redundancy_manager

#endif // REDUNDANCY_MANAGER_H
