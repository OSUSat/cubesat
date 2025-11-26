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
 * @{
 */

/**
 * @defgroup battery_management_types Structures
 * @ingroup battery_management
 * @brief Structures used by the Battery Management Service.
 * @{
 */

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
} battery_status_t;

/**
 * @struct battery_management_t
 * @brief The battery management service
 */
typedef struct {
    battery_status_t battery_status; /**< The battery status */
    bool intiialized;                /**< True if the BMS is initialized */
} battery_management_t;

/** @} */ // end battery_management_types

/**
 * @defgroup battery_management_api Public API
 * @ingroup battery_management
 * @brief External interface for interacting with the Battery Management
 * Service.
 * @{
 */

/**
 * @brief Initialize the Battery Management Service.
 *
 * This must be called once at startup before any other BMS functions.
 * Initializes internal state, and performs a startup self-check.
 *
 * @note This function is idempotent.
 *
 * @param[out] manager The battery manager
 */
void battery_init(battery_management_t *manager);

/**
 * @brief Perform a periodic update of battery status.
 *
 * This should be called once each main loop tick.
 * Reads sensors, updates SoC/SoH estimates, and runs charge/protection
 * algorithms.
 *
 * @param[in] manager The battery manager
 */
void battery_update(battery_management_t *manager);

/**
 * @brief Returns whether the battery system passed its self-check.
 *
 * @param[out] manager The battery manager
 *
 * @retval true  Battery is healthy and operational.
 * @retval false Battery failed diagnostics; EPS may require safe mode.
 */
bool battery_self_check_ok(battery_management_t *manager);

/**
 * @brief Determine if the battery requires charging.
 *
 * Uses SoC, voltage thresholds, and EPS policy rules.
 *
 * @retval true  Battery is below charging threshold.
 * @retval false Sufficient charge level or charging not allowed.
 */
bool battery_needs_charge(void);

/**
 * @brief Determine whether the battery is considered full.
 *
 * Typically derived from voltage or SoC limit.
 *
 * @retval true  Battery is full or very near full.
 * @retval false Battery still has capacity to charge.
 */
bool battery_full(void);

/**
 * @brief Apply charge-control policy.
 *
 * Enables or disables charging circuits based on SoC, temperature,
 * EPS power budget, and safety limits.
 */
void battery_charge_control(void);

/**
 * @brief Enter battery protection mode.
 *
 * Used during overvoltage, deep discharge, or
 * other critical conditions. May disable EPS rails or charging.
 */
void battery_protect_mode(void);

/**
 * @brief Get the full status snapshot of the battery system.
 *
 * @return ::battery_status_t struct with all telemetry fields populated.
 */
battery_status_t battery_get_status(void);

/**
 * @brief Get battery pack voltage.
 *
 * @return Measured voltage in volts.
 */
float battery_get_voltage(void);

/**
 * @brief Get battery pack current.
 *
 * Positive = charging, negative = discharging.
 *
 * @return Current in amps.
 */
float battery_get_current(void);

/** @} */ // end battery_management_api

/** @} */ // end battery_management

#endif // BATTERY_MANAGEMENT_H
