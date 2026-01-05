/**
 * @file redundancy_manager.h
 * @brief Centralized fault aggregation and system health monitoring.
 *
 * The Redundancy Manager subscribes to fault events from all EPS services
 * and aggregates them to determine overall system health. It acts as the
 * single source of truth for whether the satellite should enter Safe Mode
 * or handle component degradation.
 *
 * Architecture:
 * - Services publish specific fault events (e.g., BATTERY_FAULT_DETECTED)
 * - Redundancy Manager subscribes to these events
 * - Aggregates faults and evaluates system health
 * - Publishes high-level health state changes
 * - Applications monitor health events to trigger state transitions
 *
 * This eliminates the need for services to directly call redundancy manager
 * functions, maintaining loose coupling through the event bus.
 */

#ifndef REDUNDANCY_MANAGER_H
#define REDUNDANCY_MANAGER_H

#include "osusat/event_bus.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup redundancy_manager Redundancy Manager
 * @brief Aggregates component faults to determine system-wide health.
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
 * @brief Maximum number of active faults to track simultaneously.
 */
#define REDUNDANCY_MANAGER_MAX_FAULTS 16

/**
 * @brief Service Unique Identifier (16-bit).
 * Used to construct unique Event IDs.
 * 0x5366 = "Sf" (SafeMode/System Fault)
 */
#define REDUNDANCY_MANAGER_SERVICE_UID 0x5366

/**
 * @enum redundancy_manager_event_id_t
 * @brief Events published by the Redundancy Manager.
 */
typedef enum {
    /**
     * @brief Published when system enters critical health state.
     *
     * Applications should initiate Safe Mode transition.
     *
     * Payload: system_health_t (always SYSTEM_HEALTH_FAULT)
     */
    REDUNDANCY_EVENT_CRITICAL_HEALTH = 0x10,

    /**
     * @brief Published when system health becomes degraded.
     *
     * Mission can continue with reduced capability. Services may need
     * to adapt (e.g., reduce power consumption, disable non-critical features).
     *
     * Payload: system_health_t (always SYSTEM_HEALTH_DEGRADED)
     */
    REDUNDANCY_EVENT_HEALTH_DEGRADED,

    /**
     * @brief Published when system recovers to nominal health.
     *
     * All critical and degraded faults have been cleared.
     *
     * Payload: system_health_t (always SYSTEM_HEALTH_OK)
     */
    REDUNDANCY_EVENT_HEALTH_RECOVERED,

    /**
     * @brief Published when a specific component becomes degraded.
     *
     * Indicates a component failure with fallback available. Affected services
     * should switch to backup/redundant hardware.
     *
     * Example: Primary UART fails → switch to auxiliary UART
     *
     * Payload: component_degradation_t
     */
    REDUNDANCY_EVENT_COMPONENT_DEGRADED,

    /**
     * @brief Published when a degraded component recovers.
     *
     * Services may optionally switch back to primary hardware.
     *
     * Payload: component_id_t
     */
    REDUNDANCY_EVENT_COMPONENT_RECOVERED,

    /**
     * @brief Published in response to REQUEST_REDUNDANCY_HEALTH query.
     *
     * Payload: health_response_t
     */
    REDUNDANCY_EVENT_HEALTH_RESPONSE,

    /**
     * @brief Published in response to REQUEST_REDUNDANCY_COMPONENT_STATUS
     * query.
     *
     * Payload: component_status_response_t
     */
    REDUNDANCY_EVENT_COMPONENT_STATUS_RESPONSE,

    /**
     * @brief Published in response to REQUEST_REDUNDANCY_FAULT_LIST query.
     *
     * May require multiple events if fault list is large (chunked response).
     *
     * Payload: fault_list_response_t
     */
    REDUNDANCY_EVENT_FAULT_LIST_RESPONSE,

    /**
     * @brief Periodic telemetry broadcast (e.g., every 30 seconds).
     *
     * Contains summary of system health and fault counts.
     *
     * Payload: redundancy_telemetry_t
     */
    REDUNDANCY_EVENT_TELEMETRY,
} redundancy_manager_event_id_t;

#define REDUNDANCY_EVENT_CRITICAL_HEALTH                                       \
    OSUSAT_BUILD_EVENT_ID(REDUNDANCY_MANAGER_SERVICE_UID,                      \
                          REDUNDANCY_EVENT_CRITICAL_HEALTH)
#define REDUNDANCY_EVENT_HEALTH_DEGRADED                                       \
    OSUSAT_BUILD_EVENT_ID(REDUNDANCY_MANAGER_SERVICE_UID,                      \
                          REDUNDANCY_EVENT_HEALTH_DEGRADED)
#define REDUNDANCY_EVENT_HEALTH_RECOVERED                                      \
    OSUSAT_BUILD_EVENT_ID(REDUNDANCY_MANAGER_SERVICE_UID,                      \
                          REDUNDANCY_EVENT_HEALTH_RECOVERED)
#define REDUNDANCY_EVENT_COMPONENT_DEGRADED                                    \
    OSUSAT_BUILD_EVENT_ID(REDUNDANCY_MANAGER_SERVICE_UID,                      \
                          REDUNDANCY_EVENT_COMPONENT_DEGRADED)
#define REDUNDANCY_EVENT_COMPONENT_RECOVERED                                   \
    OSUSAT_BUILD_EVENT_ID(REDUNDANCY_MANAGER_SERVICE_UID,                      \
                          REDUNDANCY_EVENT_COMPONENT_RECOVERED)
#define REDUNDANCY_EVENT_HEALTH_RESPONSE                                       \
    OSUSAT_BUILD_EVENT_ID(REDUNDANCY_MANAGER_SERVICE_UID,                      \
                          REDUNDANCY_EVENT_HEALTH_RESPONSE)
#define REDUNDANCY_EVENT_COMPONENT_STATUS_RESPONSE                             \
    OSUSAT_BUILD_EVENT_ID(REDUNDANCY_MANAGER_SERVICE_UID,                      \
                          REDUNDANCY_EVENT_COMPONENT_STATUS_RESPONSE)
#define REDUNDANCY_EVENT_FAULT_LIST_RESPONSE                                   \
    OSUSAT_BUILD_EVENT_ID(REDUNDANCY_MANAGER_SERVICE_UID,                      \
                          REDUNDANCY_EVENT_FAULT_LIST_RESPONSE)
#define REDUNDANCY_EVENT_TELEMETRY                                             \
    OSUSAT_BUILD_EVENT_ID(REDUNDANCY_MANAGER_SERVICE_UID,                      \
                          REDUNDANCY_EVENT_TELEMETRY)

/**
 * @enum fault_source_t
 * @brief Identifies the subsystem reporting a failure.
 */
typedef enum {
    FAULT_SOURCE_BATTERY, /**< BMS issues (voltage, temperature, etc.) */
    FAULT_SOURCE_MPPT,    /**< Solar charging failures */
    FAULT_SOURCE_RAIL,    /**< Rail controller (overcurrent, enable failures) */
    FAULT_SOURCE_SENSOR,  /**< I2C/SPI sensor timeouts or bad data */
    FAULT_SOURCE_UART,    /**< UART communication errors */
    FAULT_SOURCE_WATCHDOG, /**< Watchdog timeout or service hang */
    FAULT_SOURCE_MEMORY,   /**< Flash/EEPROM errors */
    FAULT_SOURCE_COUNT     /**< Number of fault sources */
} fault_source_t;

/**
 * @typedef fault_code_t
 * @brief Service-specific error code.
 *
 * Specific codes are defined in the reporting service's header.
 * Example: A FAULT_SOURCE_RAIL might report RAIL_FAULT_OVERCURRENT.
 */
typedef uint32_t fault_code_t;

/**
 * @enum fault_severity_t
 * @brief Severity classification for individual faults.
 */
typedef enum {
    FAULT_SEVERITY_INFO,     /**< Informational, no action required */
    FAULT_SEVERITY_WARNING,  /**< Potential issue, monitor closely */
    FAULT_SEVERITY_DEGRADED, /**< Component degraded, fallback available */
    FAULT_SEVERITY_CRITICAL  /**< Critical failure, Safe Mode required */
} fault_severity_t;

/**
 * @enum system_health_t
 * @brief High-level classification of EPS health.
 *
 * Used by applications to drive state transitions.
 */
typedef enum {
    SYSTEM_HEALTH_OK,       /**< All systems nominal */
    SYSTEM_HEALTH_DEGRADED, /**< Non-critical faults, mission continues */
    SYSTEM_HEALTH_FAULT     /**< Critical failure, requires Safe Mode */
} system_health_t;

/**
 * @enum component_id_t
 * @brief Identifiers for components with redundancy/fallback options.
 */
typedef enum {
    COMPONENT_UART_PRIMARY,   /**< Primary UART (port 1) */
    COMPONENT_UART_SECONDARY, /**< Secondary UART (port 3) */
    COMPONENT_I2C_BUS_1,      /**< I2C bus 1 */
    COMPONENT_I2C_BUS_2,      /**< I2C bus 2 */
    COMPONENT_I2C_BUS_3,      /**< I2C bus 3 */
    COMPONENT_I2C_BUS_4,      /**< I2C bus 4 */
    COMPONENT_SOLAR_STRING_1, /**< Solar panel string 1 */
    COMPONENT_SOLAR_STRING_2, /**< Solar panel string 2 */
    COMPONENT_SOLAR_STRING_3, /**< Solar panel string 3 */
    COMPONENT_SOLAR_STRING_4, /**< Solar panel string 4 */
    COMPONENT_SOLAR_STRING_5, /**< Solar panel string 5 */
    COMPONENT_SOLAR_STRING_6, /**< Solar panel string 6 */
    COMPONENT_COUNT           /**< Number of tracked components */
} component_id_t;

/**
 * @struct fault_t
 * @brief Represents a single active fault in the system.
 */
typedef struct {
    fault_source_t source;     /**< Subsystem that reported the fault */
    fault_code_t code;         /**< Service-specific error code */
    fault_severity_t severity; /**< Severity classification */
    uint32_t timestamp_ms;     /**< When the fault was first detected */
    uint32_t count;            /**< Number of times this fault has occurred */
    bool active;               /**< True if fault is currently active */
} fault_t;

/**
 * @struct component_degradation_t
 * @brief Payload for component degradation events.
 */
typedef struct {
    component_id_t component;    /**< Which component is degraded */
    fault_source_t fault_source; /**< What caused the degradation */
    bool fallback_available; /**< True if fallback/redundant option exists */
} component_degradation_t;

/**
 * @struct health_response_t
 * @brief Response payload for health queries.
 */
typedef struct {
    system_health_t health;      /**< Current system health */
    uint32_t active_fault_count; /**< Number of active faults */
    uint32_t timestamp_ms;       /**< When health was sampled */
} health_response_t;

/**
 * @struct component_status_request_t
 * @brief Request payload to query specific component status.
 */
typedef struct {
    component_id_t component; /**< Which component to query */
} component_status_request_t;

/**
 * @struct component_status_response_t
 * @brief Response payload for component status queries.
 */
typedef struct {
    component_id_t component;    /**< Requested component */
    bool is_ok;                  /**< True if operational, false if degraded */
    fault_source_t fault_source; /**< What caused degradation (if degraded) */
    uint32_t timestamp_ms;       /**< When status was sampled */
} component_status_response_t;

/**
 * @struct fault_list_response_t
 * @brief Response payload for fault list queries.
 *
 * Contains a subset of active faults. May require multiple events
 * for complete fault list.
 */
typedef struct {
    uint32_t total_faults;    /**< Total number of active faults */
    uint32_t chunk_index;     /**< Which chunk this is (0-based) */
    uint32_t faults_in_chunk; /**< Number of faults in this response */
    fault_t faults[4];        /**< Up to 4 faults per response */
} fault_list_response_t;

/**
 * @struct redundancy_telemetry_t
 * @brief Periodic telemetry payload.
 */
typedef struct {
    system_health_t health;           /**< Current health */
    uint32_t active_fault_count;      /**< Active faults */
    uint32_t total_faults_since_boot; /**< Lifetime fault counter */
    uint32_t degraded_components;     /**< Bitmask of degraded components */
    uint32_t timestamp_ms;            /**< Telemetry timestamp */
} redundancy_telemetry_t;

/**
 * @struct redundancy_manager_t
 * @brief The redundancy manager state.
 */
typedef struct {
    fault_t faults[REDUNDANCY_MANAGER_MAX_FAULTS]; /**< Active fault list */
    system_health_t health; /**< Current aggregated system health */
    bool component_status[COMPONENT_COUNT]; /**< True = OK, False = Degraded */
    uint32_t total_fault_count; /**< Total faults since boot (for telemetry) */
    bool initialized;           /**< True if initialized */
} redundancy_manager_t;

/** @} */ // end redundancy_manager_types

/**
 * @defgroup redundancy_manager_api Public API
 * @ingroup redundancy_manager
 * @brief Functions for initializing the redundancy manager.
 *
 * All interaction with the redundancy manager occurs through events.
 * The manager publishes health updates and responds to query requests
 * via the event bus.
 *
 * @{
 */

/**
 * @brief Initialize the Redundancy Manager.
 *
 * Clears all faults, sets system health to SYSTEM_HEALTH_OK, and subscribes
 * to:
 * - Fault events from all services (e.g., BATTERY_EVENT_FAULT_DETECTED)
 * - Query request events from applications
 * - Recovery events from services
 *
 * Publishes initial REDUNDANCY_EVENT_HEALTH_RECOVERED event after init.
 *
 * @param[in,out] manager The redundancy manager to initialize.
 */
void redundancy_manager_init(redundancy_manager_t *manager);

/** @} */ // end redundancy_manager_api

/** @} */ // end redundancy_manager

#ifdef __cplusplus
}
#endif

#endif // REDUNDANCY_MANAGER_H
