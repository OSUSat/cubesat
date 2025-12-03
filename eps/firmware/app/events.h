/**
 * @file events.h
 * @brief Defines application-level events for the event bus.
 */

#ifndef APP_EVENTS_H
#define APP_EVENTS_H

#include "osusat/event_bus.h"

/**
 * @defgroup app_events Application Events
 * @brief Defines events originating from the application layer.
 *
 * @{
 */

#define APP_SERVICE_UID 0xA00 // service UID for application-level events

/**
 * @enum app_event_id_t
 * @brief Application-level event IDs.
 */
typedef enum {
    REQUEST_POWER_PROFILE_NOMINAL = 0x10,
    REQUEST_POWER_PROFILE_SAFE,
} app_event_id_t;

#define APP_EVENT_REQUEST_POWER_PROFILE_NOMINAL                                \
    OSUSAT_BUILD_EVENT_ID(APP_SERVICE_UID, REQUEST_POWER_PROFILE_NOMINAL)

#define APP_EVENT_REQUEST_POWER_PROFILE_SAFE                                   \
    OSUSAT_BUILD_EVENT_ID(APP_SERVICE_UID, REQUEST_POWER_PROFILE_SAFE)

/** @} */ // end app_events

#endif // APP_EVENTS_H
