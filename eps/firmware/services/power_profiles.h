/**
 * @file power_profiles.h
 * @brief Power Profiles abstraction layer & API.
 *
 * This service allows the application layer (Power Policies) to manage
 * groups of power rails collectively. Instead of micromanaging individual
 * rails, the application can request high-level system states like "nominal"
 * or "safe," and this service ensures the correct combination of rails is
 * enabled or disabled.
 */

#ifndef POWER_PROFILES_H
#define POWER_PROFILES_H

#include "rail_controller.h"
#include <stdbool.h> // For bool type
#include <stddef.h>

/**
 * @defgroup power_profiles Power Profiles Service
 * @brief Manages groups of power rails corresponding to system states.
 *
 * The Power Profiles service acts as a translation layer between
 * high-level system states (Safe, Nominal) and physical rails.
 *
 * It:
 *  - accepts commands to enter specific power profiles
 *  - executes the actual switch toggling for the rails defined in the power
 * profile
 *  - reads rail mappings from the system configuration.
 * @{
 */

/**
 * @defgroup power_profiles_types Types
 * @ingroup power_profiles
 * @brief Enumerations and structures used by the Power Profiles service.
 *
 * @{
 */

/**
 * @enum power_profile_t
 * @brief High-level system power configurations.
 */
typedef enum {
    POWER_PROFILE_NOMINAL, /**< Standard operation, most rails enabled */
    POWER_PROFILE_SAFE,    /**< Safe mode in case of faults or low power, most
                              rails disabled */
    // TODO: add more profiles as needed
} power_profile_t;

/**
 * @struct power_profile_info_t
 * @brief Internal container for profile configuration data.
 *
 * Used to retrieve the specific list of rails associated with a
 * requested profile.
 */
typedef struct {
    const power_rail_t *rails; /**< Pointer to the array of rails */
    int count;                 /**< Number of rails in the profile */
} power_profile_info_t;

/**
 * @enum power_profile_status_t
 * @brief Status codes for profile operations.
 */
typedef enum {
    POWER_PROFILE_SUCCESS, /**< The operation was completed successfully */
    POWER_PROFILE_ERROR_INVALID_PROFILE /**< The requested power profile does
                                           not exist */
} power_profile_status_t;

/**
 * @struct power_profiles_t
 * @brief Container for the Power Profiles Service state.
 */
typedef struct {
    rail_controller_t
        *rail_controller; /**< Pointer to the rail controller instance. */
    power_profile_t current_profile; /**< The currently active power profile. */
    bool initialized; /**< True if the service has been initialized. */
} power_profiles_t;

/** @} */ // end power_profiles_types

/**
 * @defgroup power_profiles_api Public API
 * @ingroup power_profiles
 * @brief Functions for applying power configurations.
 * @{
 */

/**
 * @brief Initialize the Power Profiles Service.
 *
 * This must be called once at startup. It subscribes to profile change
 * request events from the application layer.
 *
 * @param[in] profiles The power profiles service instance.
 * @param[in] controller The rail controller instance.
 */
void power_profiles_init(power_profiles_t *profiles,
                         rail_controller_t *controller);

/**
 * @brief Enable all power rails associated with a specific profile.
 *
 * Iterates through the list of rails defined for the given profile
 * and instructs the Rail Controller to turn them ON.
 *
 * @param[in] profiles The power profiles service instance.
 * @param[in] profile The target profile to enable (e.g.,
 * POWER_PROFILE_NOMINAL).
 *
 * @retval POWER_PROFILE_SUCCESS Operation successful.
 * @retval POWER_PROFILE_ERROR_INVALID_PROFILE The provided profile ID does not
 * exist.
 */
power_profile_status_t power_profiles_enable(power_profiles_t *profiles,
                                             power_profile_t profile);

/**
 * @brief Disable all power rails associated with a specific profile.
 *
 * Iterates through the list of rails defined for the given profile
 * and instructs the Rail Controller to turn them OFF.
 *
 * @note This is typically used when transitioning out of a state,
 * or to shut down specific subsystems associated with a mode.
 *
 * @param[in] profiles The power profiles service instance.
 * @param[in] profile The target profile to disable.
 *
 * @retval POWER_PROFILE_SUCCESS Operation successful.
 * @retval POWER_PROFILE_ERROR_INVALID_PROFILE The provided profile ID does not
 * exist.
 */
power_profile_status_t power_profiles_disable(power_profiles_t *profiles,
                                              power_profile_t profile);

/** @} */ // end power_profiles_api

/** @} */ // end power_profiles

#endif // POWER_PROFILES_H
