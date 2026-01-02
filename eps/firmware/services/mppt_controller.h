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
#include <stddef.h>
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
 *
 * @{
 */

#define MPPT_CONTROLLER_SERVICE_ID 0xAB77

typedef enum {
    /**
     * @brief Published when a critical fault is detected.
     * Payload: mppt_channel_t (Snapshot at time of fault)
     */
    MPPT_FAULT_DETECTED = 0x10,

    /**
     * @brief The state of the MPPT controller's PGOOD pin has changed
     * Payload: new state (bool)
     */
    MPPT_PGOOD_CHANGED,

    /**
     * @brief The MPPT input voltage level has lowered below the threshold
     * Payload: new voltage (float)
     */
    MPPT_VOLTAGE_LOW,

    /**
     * @brief Periodic telemetry broadcast
     * Payload: mppt_channel_t
     */
    MPPT_TELEMETRY
} mppt_controller_event_id_t;

#define MPPT_EVENT_FAULT_DETECTED                                              \
    OSUSAT_BUILD_EVENT_ID(MPPT_CONTROLLER_SERVICE_ID, MPPT_FAULT_DETECTED)
#define MPPT_EVENT_PGOOD_CHANGED                                               \
    OSUSAT_BUILD_EVENT_ID(MPPT_CONTROLLER_SERVICE_ID, MPPT_PGOOD_CHANGED)
#define MPPT_EVENT_VOLTAGE_LOW                                                 \
    OSUSAT_BUILD_EVENT_ID(MPPT_CONTROLLER_SERVICE_ID, MPPT_VOLTAGE_LOW)
#define MPPT_EVENT_TELEMETRY                                                   \
    OSUSAT_BUILD_EVENT_ID(MPPT_CONTROLLER_SERVICE_ID, MPPT_TELEMETRY)

/**
 * @enum mppt_status_t
 * @brief MPPT channel health state.
 *
 * Represents the operating or fault condition of a solar MPPT channel.
 */
typedef enum {
    MPPT_STATUS_OK,        /**< Channel operating nominally */
    MPPT_STATUS_DISABLED,  /**< Channel disabled by software or hardware */
    MPPT_STATUS_FAULT,     /**< General hardware fault or unexpected condition
                            */
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
    size_t num_channels;      /**< Number of MPPT channels */
    uint32_t tick_counter;    /**< Internal counter for update loop */
    uint32_t telemetry_tick_counter; /**< Internal counter for telemetry publish
                                        timing */
    bool initialized; /**< True if this device has been initialized */
} mppt_t;

/** @} */ // end mppt_controller_types

/**
 * @defgroup mppt_controller_api Public API
 * @ingroup mppt_controller
 * @brief External interface for interacting with the MPPT Controller Service.
 *
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
 * @note If called more than once, the internal MPPT controller state will be
 * reset.
 *
 * @param[out] mppt The MPPT controller
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

/** @} */ // end mppt_controller_api
/** @} */ // end mppt_controller

#endif
