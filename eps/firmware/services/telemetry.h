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

#include "battery_management.h"
#include "can_events.h"
#include "mppt_controller.h"
#include "rail_controller.h"
#include "redundancy_manager.h"
#include "uart_events.h"
#include <stdbool.h>
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

typedef struct {
    uint32_t rx_byte_count;      /**< Total received bytes */
    uint32_t rx_packet_count;    /**< Total packets decoded */
    uint32_t rx_crc_error_count; /**< Total errors counted */
    bool initialized;            /**< True if initialized */
} uart_telemetry_t;

typedef struct {
    uint32_t rx_byte_count;      /**< Total received bytes */
    uint32_t rx_packet_count;    /**< Total packets decoded */
    uint32_t rx_crc_error_count; /**< Total errors counted */
    bool initialized;            /**< True if initialized */
} can_telemetry_t;

/**
 * @struct eps_telemetry_t
 * @brief The Master Telemetry Packet.
 *
 * This structure contains a snapshot of the entire EPS state.
 * It may include nested structures from other services.
 */
typedef struct {
    battery_status_t battery; /**< BMS status telemetry */
    mppt_channel_t
        mppt_channels[NUM_MPPT_CHANNELS]; /**< Solar/MPPT channels telemetry */
    rail_t rails[NUM_POWER_RAILS];        /**< Power rails telemetry */
    redundancy_telemetry_t redundancy;    /**< Redundancy manager telemetry */
    uart_telemetry_t uart1;               /**< USART1 telemetry */
    uart_telemetry_t uart3;               /**< USART3 telemetry */
    can_telemetry_t can1;                 /**< CAN1 telemetry */
    can_telemetry_t can2;                 /**< CAN2 telemetry */
    float v_reg_5v;                       /**< 5V regulator voltage in V */
    float v_reg_3v3;                      /**< 3.3V regulator voltage in V */
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
    const battery_management_t *battery_manager;
    const mppt_t *mppt_controller;
    const rail_controller_t *rail_controller;
    const redundancy_manager_t *redundancy_manager;
    const uart_events_t *usart1_events;
    const uart_events_t *usart3_events;
    const can_events_t *can1_events;
    const can_events_t *can2_events;

    uint32_t
        tick_counter; /**< Internal counter for timing telemetry collection */
    bool initialized; /**< Whether the telemetry service has been initialized */
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
