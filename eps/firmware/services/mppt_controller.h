/**
 * @file mppt_controller.h
 * @brief MPPT (Maximum Power Point Tracking) Controller Service public API.
 *
 * This service manages the EPS’s MPPT hardware, providing:
 *   - chip initialization and configuration
 *   - enable/disable interfaces for MPPT channels
 *   - input/output voltage & current sensing
 *   - status monitoring (PGOOD, faults, thermal limits)
 *   - telemetry packaging for the higher-level power policy app
 */

#ifndef MPPT_CONTROLLER_H
#define MPPT_CONTROLLER_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @defgroup mppt_controller MPPT Controller Service
 * @brief Manages solar MPPT channels and associated telemetry.
 *
 * The MPPT controller handles all solar power input channels. Responsibilities:
 *  - initialize MPPT ICs and configure GPIO/HAL interfaces
 *  - read per-channel voltage/current measurements
 *  - monitor MPPT status (faults, overtemp, undervoltage)
 *  - compute per-channel and total solar input power
 *  - provide interfaces to higher-level services (battery, power policy)
 *
 * @{
 */

/**
 * @defgroup mppt_controller_types Structures & Enums
 * @ingroup mppt_controller
 * @brief Types used by the MPPT Controller Service.
 * @{
 */

/**
 * @enum mppt_status_t
 * @brief MPPT channel health state.
 *
 * Represents the operating or fault condition of a solar MPPT channel.
 */
typedef enum {
    MPPT_STATUS_OK,       /**< Channel operating nominally */
    MPPT_STATUS_DISABLED, /**< Channel disabled by software or hardware */
    MPPT_STATUS_FAULT,    /**< General hardware fault or unexpected condition */
    MPPT_STATUS_UNDERVOLT, /**< Input voltage too low for MPPT to operate */
    MPPT_STATUS_OVERTEMP   /**< Channel is thermally throttled or shut down */
} mppt_status_t;

/**
 * @struct mppt_channel_t
 * @brief Snapshot of an individual MPPT channel.
 *
 * Each channel includes voltage/current on both input and output sides,
 * power computation, and MPPT chip status.
 */
typedef struct {
    float input_voltage;  /**< Input voltage (solar panel side), in volts */
    float input_current;  /**< Input current, in amps */
    float output_voltage; /**< Regulated output voltage, in volts */
    float output_current; /**< Output current, in amps */
    float power;          /**< Computed output power, in watts */
    mppt_status_t status; /**< Operational status */
    bool enabled;         /**< True if channel is enabled */
    bool pgood;           /**< True if MPPT chip reports power-good */
} mppt_channel_t;

/**
 * @struct mppt_t
 * @brief Individual MPPT device
 *
 * Each MPPT device must be initialized with ::mppt_init
 */
typedef struct {
    mppt_channel_t *channels; /**< List of MPPT channels for this device */
    bool initialized;         /**< True if this device has been initialized */
} mppt_t;

/** @} */ // end mppt_controller_types

/**
 * @defgroup mppt_controller_api Public API
 * @ingroup mppt_controller
 * @brief External interface for interacting with the MPPT Controller Service.
 * @{
 */

/**
 * @brief Initialize all MPPT channels.
 *
 * Configures MPPT hardware interfaces & chips, sets default states,
 * and prepares for voltage/current sensing.
 *
 * This must be called once at startup per MPPT device before any other
 * MPPT functions are called.
 *
 * @note This function is idempotent.
 */
void mppt_init(mppt_t *mppt);

/**
 * @brief Enable a specific MPPT channel.
 *
 * @param ch Channel index (0 to NUM_MPPT_CHANNELS-1)
 *
 * Enables the MPPT chip via GPIO, clears stale faults,
 * and marks the channel active for telemetry updates.
 */
void mppt_enable(uint8_t ch);

/**
 * @brief Disable a specific MPPT channel.
 *
 * @param ch Channel index (0 to NUM_MPPT_CHANNELS-1)
 *
 * Used during power-budget constraints, safe-mode transitions,
 * or thermal protection events.
 */
void mppt_disable(uint8_t ch);

/**
 * @brief Update readings for all MPPT channels.
 *
 * Clears and refreshes telemetry for each MPPT IC:
 *   - input/output voltage/current
 *   - computed power
 *   - PGOOD & fault state
 *   - thermal/undervoltage status
 *
 * Should be called once per main loop cycle.
 */
void mppt_update(void);

/**
 * @brief Get the current telemetry for a given MPPT channel.
 *
 * @param ch Channel index (0 to NUM_MPPT_CHANNELS-1)
 * @return ::mppt_channel_t snapshot of the channel state.
 */
mppt_channel_t mppt_get_channel(uint8_t ch);

/**
 * @brief Get total power generated across all MPPT channels.
 *
 * Aggregates the `power` field from all enabled channels.
 *
 * @return Total power in watts.
 */
float mppt_get_total_power(void);

/** @} */ // end mppt_controller_api
/** @} */ // end mppt_controller

#endif
