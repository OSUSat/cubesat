/**
 * @file battery_management.h
 * @brief Battery Management Service (BMS) public API.
 *
 * This service manages high-level battery behavior for the EPS, including:
 *  - pack voltage/current monitoring
 *  - temperature and health estimation
 *  - charge state determination
 *  - enabling/disabling charging/balancing circuits
 *  - protection and failsafe behavior
 *  - telemetry collection for upstream consumption
 *  - error state management by notifying redundancy manager
 *
 *  The BMS interacts with HAL drivers to obtain sensor readings,
 *  and provides state for use by the power policy application.
 */

#ifndef BATTERY_MANAGEMENT_H
#define BATTERY_MANAGEMENT_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @defgroup battery_management Battery Management Service
 * @brief Monitors and controls the CubeSat battery pack.
 *
 * The BMS is responsible for:
 *  - pack voltage/current monitoring
 *  - temperature and health estimation
 *  - charge state determination
 *  - enabling/disabling charging/balancing circuits
 *  - protection and failsafe behavior
 *  - telemetry collection for upstream consumption
 *  - error state management by notifying redundancy manager
 *
 * @{
 */

/**
 * @defgroup battery_management_types Structures
 * @ingroup battery_management
 * @brief Structures used by the Battery Management Service.
 *
 * @{
 */

/**
 * @brief Service Unique Identifier (16-bit).
 * Used to construct unique Event IDs.
 * "BA77" = BATT
 */
#define BATTERY_SERVICE_UID 0xBA77

typedef enum {
    /**
     * @brief Published when a critical fault is detected.
     * Payload: battery_status_t (Snapshot at time of failure)
     */
    BATTERY_FAULT_DETECTED = 0x10,

    /**
     * @brief Published when the battery management service passes its
     * self-check. Payload: NULL
     */
    BATTERY_SELF_CHECK_PASSED,

    /**
     * @brief Published when the battery management service fails its
     * self-check. Payload: failure mode
     */
    BATTERY_SELF_CHECK_FAILED,

    /**
     * @brief Published when voltage drops below critical threshold.
     * Payload: float (Current Voltage)
     */
    BATTERY_CRITICAL_LOW,

    /**
     * @brief Published when charging starts or stops.
     * Payload: bool (true = charging started, false = stopped)
     */
    BATTERY_CHARGING_CHANGE,

    /**
     * @brief Published when battery reaches 100% SoC.
     * Payload: NULL
     */
    BATTERY_FULLY_CHARGED,

    /**
     * @brief Periodic telemetry broadcast (e.g., every 10s or 1 min).
     * Payload: battery_status_t
     */
    BATTERY_TELEMETRY
} battery_event_id_t;

#define BATTERY_EVENT_FAULT_DETECTED                                           \
    OSUSAT_BUILD_EVENT_ID(BATTERY_SERVICE_UID, BATTERY_FAULT_DETECTED)
#define BATTERY_EVENT_SELF_CHECK_PASSED                                        \
    OSUSAT_BUILD_EVENT_ID(BATTERY_SERVICE_UID, BATTERY_SELF_CHECK_PASSED)
#define BATTERY_EVENT_SELF_CHECK_FAILED                                        \
    OSUSAT_BUILD_EVENT_ID(BATTERY_SERVICE_UID, BATTERY_SELF_CHECK_FAILED)
#define BATTERY_EVENT_CRITICAL_LOW                                             \
    OSUSAT_BUILD_EVENT_ID(BATTERY_SERVICE_UID, BATTERY_CRITICAL_LOW)
#define BATTERY_EVENT_CHARGING_CHANGE                                          \
    OSUSAT_BUILD_EVENT_ID(BATTERY_SERVICE_UID, BATTERY_CHARGING_CHANGE)
#define BATTERY_EVENT_FULLY_CHARGED                                            \
    OSUSAT_BUILD_EVENT_ID(BATTERY_SERVICE_UID, BATTERY_FULLY_CHARGED)
#define BATTERY_EVENT_TELEMETRY                                                \
    OSUSAT_BUILD_EVENT_ID(BATTERY_SERVICE_UID, BATTERY_TELEMETRY)

/**
 * @struct battery_status_t
 * @brief Snapshot of system battery state
 *
 * This structure is returned by ::battery_get_status and is
 * used internally by the BMS to make decisions.
 */
typedef struct {
    float voltage; /**< Current pack voltage in volts */
    float current; /**< Pack current in amps (+ = charging, - = discharging) */
    float temperature; /**< Average pack temperature in C */
    float soc;         /**< State of charge (0-100%) */
    float soh;         /**< State of health estimate (0-100%) */
    bool charging;     /**< True if charging is currently active */
    bool balancing;    /**< True if balancing circuits are enabled */
    bool protection; /**< True if in battery protection mode (could be due to a
                        fault, etc.) */
} battery_status_t;

/**
 * @struct battery_management_t
 * @brief The battery management service
 */
typedef struct {
    battery_status_t battery_status; /**< The battery status */
    bool initialized;                /**< True if the BMS is initialized */
} battery_management_t;

/** @} */ // end battery_management_types

/**
 * @defgroup battery_management_api Public API
 * @ingroup battery_management
 * @brief External interface for interacting with the Battery Management
 * Service.
 *
 * @{
 */

/**
 * @brief Initialize the Battery Management Service.
 *
 * This must be called once at startup before any other BMS functions.
 * Initializes internal state, and performs a startup self-check.
 *
 * @note If called more than once, the internal battery state will be reset.
 *
 * @param[out] manager The battery manager
 */
void battery_init(battery_management_t *manager);

/**
 * @brief Apply charge-control policy.
 *
 * Enables or disables charging circuits based on SoC, temperature,
 * EPS power budget, and safety limits.
 *
 * @param[in] manager The battery manager
 */
void battery_charge_control(battery_management_t *manager, bool enable);

/**
 * @brief Enter battery protection mode.
 *
 * Used during overvoltage, deep discharge, or
 * other critical conditions. May disable EPS rails or charging.
 *
 * @param[in] manager The battery manager
 */
void battery_protect_mode(battery_management_t *manager);

/** @} */ // end battery_management_api

/** @} */ // end battery_management

#endif // BATTERY_MANAGEMENT_H
