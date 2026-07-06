/**
 * @file power_policies.h
 * @brief Application layer for power management state machine.
 */

#ifndef POWER_POLICIES_H
#define POWER_POLICIES_H

#include "power_profiles.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * @defgroup power_policies Power Policies Application
 * @brief Manages the overall power state of the EPS based on system conditions.
 *
 * This application is responsible for:
 *  - Monitoring system health and battery status.
 *  - Transitioning between different power profiles (e.g., Nominal, Safe).
 *  - Implementing high-level power management logic.
 * @{
 */

/**
 * @defgroup power_policies_types Types
 * @ingroup power_policies
 * @brief Structures used by the Power Policies Application.
 * @{
 */

/**
 * @struct power_policies_t
 * @brief State container for the power policies application.
 */
typedef struct {
    power_profile_t current_profile; /**< The current power profile. */
    bool initialized; /**< True if the application is initialized. */
} power_policies_t;

/** @} */ // end power_policies_types

/**
 * @defgroup power_policies_api Public API
 * @ingroup power_policies
 * @brief Functions for managing the power policies application.
 * @{
 */

/**
 * @brief Initialize the power policies application.
 *
 * @param[out] app The power policies application to initialize.
 */
void power_policies_init(power_policies_t *app);

/** @} */ // end power_policies_api

/** @} */ // end power_policies

#endif // POWER_POLICIES_H
