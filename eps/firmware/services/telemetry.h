/**
 * @file telemetry.h
 * @brief Centralized telemetry aggregation service.
 *
 * This module acts as the "reporter" for the EPS. It gathers data from
 * all other functional services (Battery, Rails, MPPT, Redundancy)
 * and packages them into a single structure.
 *
 * This unified structure is used to:
 * - Serialize and send data to the OBC.
 * - Make high-level system decisions based on global state.
 */

#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <stdint.h>

/**
 * @defgroup telemetry Telemetry Service
 * @brief Collects, organizes, and exposes system-wide data.
 *
 * The Telemetry service decouples data collection from data transmission.
 * It ensures that when the OBC requests a status update, the data is
 * already aggregated and ready to read.
 *
 * @{
 */

/**
 * @defgroup telemetry_types Types
 * @ingroup telemetry
 * @brief Types for telemetry.
 *
 * @{
 */

/**
 * @struct eps_telemetry_t
 * @brief The Master Telemetry Packet.
 *
 * This structure contains a snapshot of the entire EPS state.
 * It may include nested structures from other services.
 */
typedef struct {
    // TODO: this will be a larger struct containing all service telemetry data
} eps_telemetry_t;

/**
 * @struct telemetry_t
 * @brief Service state container.
 *
 * Holds the actual instance data for the telemetry service, including
 * the current snapshot of the system state.
 */
typedef struct {
    eps_telemetry_t telemetry; /**< The current aggregated data snapshot. */
} telemetry_t;

/** @} */ // end telemetry_types

/**
 * @defgroup telemetry_api Public API
 * @ingroup telemetry
 *
 * @brief Functions for updating and retrieving telemetry.
 *
 * @{
 */

/**
 * @brief Initialize the Telemetry aggregator instance.
 *
 * Prepares the internal data structures and zeroes out the
 * telemetry packet within the provided handle.
 *
 * @param[out] telemetry Pointer to the telemetry service instance to
 * initialize.
 */
void telemetry_init(telemetry_t *telemetry);

/**
 * @brief Periodic update task.
 *
 * This function queries all other active services (Battery, Rails, etc.)
 * to get their latest status and updates the snapshot inside the provided
 * handle.
 *
 * @param[in,out] telemetry Pointer to the telemetry service instance to update.
 */
void telemetry_update(telemetry_t *telemetry);

/**
 * @brief Retrieve the latest full system snapshot.
 *
 * @param[in] telemetry Pointer to the telemetry service instance to read from.
 * @return ::eps_telemetry_t A copy of the aggregated telemetry data.
 */
eps_telemetry_t telemetry_get_all(telemetry_t *telemetry);

/** @} */ // end telemetry_api

/** @} */ // end telemetry

#endif // TELEMETRY_H
