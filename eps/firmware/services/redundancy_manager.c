/**
 * @file redundancy_manager.c
 * @brief Redundancy Manager Service Implementation
 */

#include "redundancy_manager.h"
#include "battery_management.h"
#include "events.h"
#include "hal_time.h"
#include "logging.h"
#include "osusat/event_bus.h"
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#define REDUNDANCY_MANAGER_UPDATE_INTERVAL_TICKS 100
#define TELEMETRY_INTERVAL_CYCLES 300

/**
 * @brief System Tick Handler.
 * Called automatically by the Event Bus.
 *
 * @param e   The event (SYSTICK).
 * @param ctx The context pointer (points to redundancy_manager_t).
 */
static void redundancy_manager_handle_tick(const osusat_event_t *e, void *ctx);

/**
 * @brief Request Handler.
 * Processes application requests for health/component status.
 *
 * @param e   The request event.
 * @param ctx The context pointer (points to redundancy_manager_t).
 */
static void redundancy_manager_handle_request(const osusat_event_t *e,
                                              void *ctx);

/**
 * @brief Battery Fault Handler.
 * Called when battery service publishes fault events.
 *
 * @param e   The fault event.
 * @param ctx The context pointer (points to redundancy_manager_t).
 */
static void redundancy_manager_handle_battery_fault(const osusat_event_t *e,
                                                    void *ctx);

/**
 * @brief MPPT Fault Handler.
 * Called when MPPT service publishes fault events.
 *
 * @param e   The fault event.
 * @param ctx The context pointer (points to redundancy_manager_t).
 */
static void redundancy_manager_handle_mppt_fault(const osusat_event_t *e,
                                                 void *ctx);

/**
 * @brief Rail Controller Fault Handler.
 * Called when rail controller publishes fault events.
 *
 * @param e   The fault event.
 * @param ctx The context pointer (points to redundancy_manager_t).
 */
static void redundancy_manager_handle_rail_fault(const osusat_event_t *e,
                                                 void *ctx);

/**
 * @brief UART Fault Handler.
 * Called when UART HAL detects communication errors.
 *
 * @param e   The fault event.
 * @param ctx The context pointer (points to redundancy_manager_t).
 */
static void redundancy_manager_handle_uart_fault(const osusat_event_t *e,
                                                 void *ctx);

/**
 * @brief Add a fault to the active fault list.
 *
 * @param manager  The redundancy manager.
 * @param source   Fault source.
 * @param code     Fault code.
 * @param severity Fault severity.
 */
static void redundancy_manager_add_fault(redundancy_manager_t *manager,
                                         fault_source_t source,
                                         fault_code_t code,
                                         fault_severity_t severity);

/**
 * @brief Remove a fault from the active fault list.
 *
 * @param manager The redundancy manager.
 * @param source  Fault source.
 * @param code    Fault code.
 * @return True if fault was found and removed.
 */
static bool redundancy_manager_remove_fault(redundancy_manager_t *manager,
                                            fault_source_t source,
                                            fault_code_t code);

/**
 * @brief Evaluate overall system health based on active faults.
 *
 * @param manager The redundancy manager.
 * @return Calculated system health state.
 */
static system_health_t
redundancy_manager_evaluate_health(const redundancy_manager_t *manager);

/**
 * @brief Publish health state change event.
 *
 * @param manager    The redundancy manager.
 * @param new_health The new health state.
 */
static void
redundancy_manager_publish_health_change(redundancy_manager_t *manager,
                                         system_health_t new_health);

/**
 * @brief Publish component degradation event.
 *
 * @param component     Component that degraded.
 * @param fault_source  What caused the degradation.
 * @param has_fallback  True if fallback is available.
 */
static void redundancy_manager_publish_component_degradation(
    component_id_t component, fault_source_t fault_source, bool has_fallback);

/**
 * @brief Publish telemetry snapshot.
 *
 * @param manager The redundancy manager.
 */
static void
redundancy_manager_publish_telemetry(const redundancy_manager_t *manager);

void redundancy_manager_init(redundancy_manager_t *manager) {
    if (manager == NULL) {
        return;
    }

    memset(manager, 0, sizeof(redundancy_manager_t));

    manager->health = SYSTEM_HEALTH_OK;
    manager->initialized = true;

    for (size_t i = 0; i < COMPONENT_COUNT; i++) {
        manager->component_status[i] = true;
    }

    osusat_event_bus_subscribe(EVENT_SYSTICK, redundancy_manager_handle_tick,
                               manager);

    osusat_event_bus_subscribe(APP_EVENT_REQUEST_REDUNDANCY_HEALTH,
                               redundancy_manager_handle_request, manager);
    osusat_event_bus_subscribe(APP_EVENT_REQUEST_REDUNDANCY_COMPONENT_STATUS,
                               redundancy_manager_handle_request, manager);
    osusat_event_bus_subscribe(APP_EVENT_REQUEST_REDUNDANCY_FAULT_LIST,
                               redundancy_manager_handle_request, manager);
    osusat_event_bus_subscribe(APP_EVENT_REQUEST_REDUNDANCY_CLEAR_FAULT,
                               redundancy_manager_handle_request, manager);
    osusat_event_bus_subscribe(APP_EVENT_REQUEST_REDUNDANCY_CLEAR_ALL,
                               redundancy_manager_handle_request, manager);

    osusat_event_bus_subscribe(BATTERY_EVENT_FAULT_DETECTED,
                               redundancy_manager_handle_battery_fault,
                               manager);
    osusat_event_bus_subscribe(BATTERY_EVENT_CRITICAL_LOW,
                               redundancy_manager_handle_battery_fault,
                               manager);

    // TODO: Subscribe to MPPT fault events when defined
    // osusat_event_bus_subscribe(MPPT_EVENT_FAULT,
    //                            redundancy_manager_handle_mppt_fault,
    //                            manager);
    // TODO: Subscribe to Rail Controller fault events when defined
    // osusat_event_bus_subscribe(RAIL_EVENT_OVERCURRENT,
    //                            redundancy_manager_handle_rail_fault,
    //                            manager);
    // TODO: Subscribe to UART fault events when defined
    // osusat_event_bus_subscribe(UART_EVENT_TRANSMISSION_FAILED,
    //                            redundancy_manager_handle_uart_fault,
    //                            manager);

    LOG_INFO(EPS_COMPONENT_MAIN, "Redundancy manager initialized");

    osusat_event_bus_publish(REDUNDANCY_EVENT_HEALTH_RECOVERED,
                             &manager->health, sizeof(manager->health));
}

static void redundancy_manager_handle_tick(const osusat_event_t *e, void *ctx) {
    (void)e;
    redundancy_manager_t *manager = (redundancy_manager_t *)ctx;

    if (manager == NULL || !manager->initialized) {
        return;
    }

    static uint32_t telemetry_counter = 0;
    telemetry_counter++;

    if (telemetry_counter >= TELEMETRY_INTERVAL_CYCLES) {
        telemetry_counter = 0;
        redundancy_manager_publish_telemetry(manager);
    }
}

static void redundancy_manager_handle_request(const osusat_event_t *e,
                                              void *ctx) {
    redundancy_manager_t *manager = (redundancy_manager_t *)ctx;

    if (manager == NULL || !manager->initialized) {
        return;
    }

    switch (e->id) {
    case APP_EVENT_REQUEST_REDUNDANCY_HEALTH: {
        health_response_t response = {.health = manager->health,
                                      .active_fault_count =
                                          0, // must count them
                                      .timestamp_ms = hal_time_get_ms()};

        for (size_t i = 0; i < REDUNDANCY_MANAGER_MAX_FAULTS; i++) {
            if (manager->faults[i].active) {
                response.active_fault_count++;
            }
        }

        osusat_event_bus_publish(REDUNDANCY_EVENT_HEALTH_RESPONSE, &response,
                                 sizeof(response));

        break;
    }

    case APP_EVENT_REQUEST_REDUNDANCY_COMPONENT_STATUS: {
        if (e->payload_len < sizeof(component_status_request_t)) {
            return;
        }

        component_status_request_t *req =
            (component_status_request_t *)e->payload;

        if (req->component >= COMPONENT_COUNT) {
            return;
        }

        component_status_response_t response = {
            .component = req->component,
            .is_ok = manager->component_status[req->component],
            .fault_source = FAULT_SOURCE_COUNT, // no fault if OK
            .timestamp_ms = hal_time_get_ms()};

        if (!response.is_ok) {
            for (size_t i = 0; i < REDUNDANCY_MANAGER_MAX_FAULTS; i++) {
                if (manager->faults[i].active) {
                    // TODO: map fault to component
                    response.fault_source = manager->faults[i].source;
                }
            }
        }

        osusat_event_bus_publish(REDUNDANCY_EVENT_COMPONENT_STATUS_RESPONSE,
                                 &response, sizeof(response));
        break;
    }

    case APP_EVENT_REQUEST_REDUNDANCY_FAULT_LIST: {
        uint32_t active_count = 0;

        for (size_t i = 0; i < REDUNDANCY_MANAGER_MAX_FAULTS; i++) {
            if (manager->faults[i].active) {
                active_count++;
            }
        }

        // send faults in chunks (max 4 per response)
        fault_list_response_t response = {.total_faults = active_count,
                                          .chunk_index = 0,
                                          .faults_in_chunk = 0};

        for (size_t i = 0; i < REDUNDANCY_MANAGER_MAX_FAULTS; i++) {
            if (manager->faults[i].active) {
                response.faults[response.faults_in_chunk++] =
                    manager->faults[i];

                if (response.faults_in_chunk >= 4) {
                    osusat_event_bus_publish(
                        REDUNDANCY_EVENT_FAULT_LIST_RESPONSE, &response,
                        sizeof(response));

                    response.chunk_index++;
                    response.faults_in_chunk = 0;
                }
            }
        }

        // send remaining faults (if any)
        if (response.faults_in_chunk > 0) {
            osusat_event_bus_publish(REDUNDANCY_EVENT_FAULT_LIST_RESPONSE,
                                     &response, sizeof(response));
        }
        break;
    }

    case APP_EVENT_REQUEST_REDUNDANCY_CLEAR_FAULT: {
        if (e->payload_len < sizeof(fault_t)) {
            return;
        }

        fault_t *fault = (fault_t *)e->payload;

        bool cleared = redundancy_manager_remove_fault(manager, fault->source,
                                                       fault->code);

        if (cleared) {
            LOG_INFO(EPS_COMPONENT_MAIN, "Fault cleared: src=%d code=0x%08X",
                     fault->source, fault->code);

            // re-evaluate health
            system_health_t new_health =
                redundancy_manager_evaluate_health(manager);

            if (new_health != manager->health) {
                redundancy_manager_publish_health_change(manager, new_health);
            }
        }
        break;
    }

    case APP_EVENT_REQUEST_REDUNDANCY_CLEAR_ALL: {
        for (size_t i = 0; i < REDUNDANCY_MANAGER_MAX_FAULTS; i++) {
            manager->faults[i].active = false;
        }

        LOG_WARN(EPS_COMPONENT_MAIN, "All faults cleared (manual)");

        // reset health to OK
        system_health_t new_health = SYSTEM_HEALTH_OK;

        if (new_health != manager->health) {
            redundancy_manager_publish_health_change(manager, new_health);
        }

        break;
    }

    default:
        break;
    }
}

static void redundancy_manager_handle_battery_fault(const osusat_event_t *e,
                                                    void *ctx) {
    redundancy_manager_t *manager = (redundancy_manager_t *)ctx;

    // extract fault code from event ID
    fault_code_t code = OSUSAT_GET_LOCAL_CODE(e->id);
    fault_severity_t severity = FAULT_SEVERITY_WARNING;

    // determine severity based on event type
    if (e->id == BATTERY_EVENT_CRITICAL_LOW) {
        severity = FAULT_SEVERITY_CRITICAL;
    } else if (e->id == BATTERY_EVENT_FAULT_DETECTED) {
        severity = FAULT_SEVERITY_DEGRADED;
    }

    redundancy_manager_add_fault(manager, FAULT_SOURCE_BATTERY, code, severity);

    LOG_WARN(EPS_COMPONENT_MAIN, "Battery fault: code=0x%08X severity=%d", code,
             severity);

    // re-evaluate system health
    system_health_t new_health = redundancy_manager_evaluate_health(manager);

    if (new_health != manager->health) {
        redundancy_manager_publish_health_change(manager, new_health);
    }
}

static void redundancy_manager_handle_mppt_fault(const osusat_event_t *e,
                                                 void *ctx) {
    redundancy_manager_t *manager = (redundancy_manager_t *)ctx;

    fault_code_t code = OSUSAT_GET_LOCAL_CODE(e->id);
    redundancy_manager_add_fault(manager, FAULT_SOURCE_MPPT, code,
                                 FAULT_SEVERITY_DEGRADED);

    system_health_t new_health = redundancy_manager_evaluate_health(manager);

    if (new_health != manager->health) {
        redundancy_manager_publish_health_change(manager, new_health);
    }
}

static void redundancy_manager_handle_rail_fault(const osusat_event_t *e,
                                                 void *ctx) {
    redundancy_manager_t *manager = (redundancy_manager_t *)ctx;

    fault_code_t code = OSUSAT_GET_LOCAL_CODE(e->id);
    redundancy_manager_add_fault(manager, FAULT_SOURCE_RAIL, code,
                                 FAULT_SEVERITY_DEGRADED);

    system_health_t new_health = redundancy_manager_evaluate_health(manager);

    if (new_health != manager->health) {
        redundancy_manager_publish_health_change(manager, new_health);
    }
}

static void redundancy_manager_handle_uart_fault(const osusat_event_t *e,
                                                 void *ctx) {
    redundancy_manager_t *manager = (redundancy_manager_t *)ctx;

    // extract UART port from payload
    if (e->payload_len < 1) {
        return;
    }

    uint8_t port = e->payload[0];
    fault_code_t code = (port << 8) | OSUSAT_GET_LOCAL_CODE(e->id);

    redundancy_manager_add_fault(manager, FAULT_SOURCE_UART, code,
                                 FAULT_SEVERITY_DEGRADED);

    // publish component degradation for UART
    component_id_t component =
        (port == 1) ? COMPONENT_UART_PRIMARY : COMPONENT_UART_SECONDARY;
    bool fallback_available = true; // we have 2 UARTs

    redundancy_manager_publish_component_degradation(
        component, FAULT_SOURCE_UART, fallback_available);

    // mark component as degraded
    manager->component_status[component] = false;

    LOG_WARN(EPS_COMPONENT_COMMS, "UART%d fault detected", port);
}

static void redundancy_manager_add_fault(redundancy_manager_t *manager,
                                         fault_source_t source,
                                         fault_code_t code,
                                         fault_severity_t severity) {
    if (manager == NULL) {
        return;
    }

    // check if fault already exists
    for (size_t i = 0; i < REDUNDANCY_MANAGER_MAX_FAULTS; i++) {
        if (manager->faults[i].active && manager->faults[i].source == source &&
            manager->faults[i].code == code) {
            // increment occurrence count
            manager->faults[i].count++;

            return;
        }
    }

    // find empty slot
    for (size_t i = 0; i < REDUNDANCY_MANAGER_MAX_FAULTS; i++) {
        if (!manager->faults[i].active) {
            manager->faults[i].source = source;
            manager->faults[i].code = code;
            manager->faults[i].severity = severity;
            manager->faults[i].timestamp_ms = hal_time_get_ms();
            manager->faults[i].count = 1;
            manager->faults[i].active = true;
            manager->total_fault_count++;

            return;
        }
    }

    // fault table full - log error
    LOG_ERROR(EPS_COMPONENT_MAIN, "Fault table full, cannot add fault");
}

static bool redundancy_manager_remove_fault(redundancy_manager_t *manager,
                                            fault_source_t source,
                                            fault_code_t code) {
    if (manager == NULL) {
        return false;
    }

    for (size_t i = 0; i < REDUNDANCY_MANAGER_MAX_FAULTS; i++) {
        if (manager->faults[i].active && manager->faults[i].source == source &&
            manager->faults[i].code == code) {
            manager->faults[i].active = false;
            return true;
        }
    }

    return false;
}

static system_health_t
redundancy_manager_evaluate_health(const redundancy_manager_t *manager) {
    if (manager == NULL) {
        return SYSTEM_HEALTH_FAULT;
    }

    bool has_critical = false;
    bool has_degraded = false;

    for (size_t i = 0; i < REDUNDANCY_MANAGER_MAX_FAULTS; i++) {
        if (manager->faults[i].active) {
            if (manager->faults[i].severity == FAULT_SEVERITY_CRITICAL) {
                has_critical = true;
                break;
            } else if (manager->faults[i].severity == FAULT_SEVERITY_DEGRADED) {
                has_degraded = true;
            }
        }
    }

    if (has_critical) {
        return SYSTEM_HEALTH_FAULT;
    } else if (has_degraded) {
        return SYSTEM_HEALTH_DEGRADED;
    }

    return SYSTEM_HEALTH_OK;
}

static void
redundancy_manager_publish_health_change(redundancy_manager_t *manager,
                                         system_health_t new_health) {
    manager->health = new_health;

    osusat_event_id_t event_id;
    const char *health_str;

    switch (new_health) {
    case SYSTEM_HEALTH_FAULT:
        event_id = REDUNDANCY_EVENT_CRITICAL_HEALTH;
        health_str = "FAULT";
        break;
    case SYSTEM_HEALTH_DEGRADED:
        event_id = REDUNDANCY_EVENT_HEALTH_DEGRADED;
        health_str = "DEGRADED";
        break;
    case SYSTEM_HEALTH_OK:
        event_id = REDUNDANCY_EVENT_HEALTH_RECOVERED;
        health_str = "OK";
        break;
    default:
        return;
    }

    LOG_INFO(EPS_COMPONENT_MAIN, "System health changed to %s", health_str);

    osusat_event_bus_publish(event_id, &new_health, sizeof(new_health));
}

static void redundancy_manager_publish_component_degradation(
    component_id_t component, fault_source_t fault_source, bool has_fallback) {
    component_degradation_t degradation = {.component = component,
                                           .fault_source = fault_source,
                                           .fallback_available = has_fallback};

    osusat_event_bus_publish(REDUNDANCY_EVENT_COMPONENT_DEGRADED, &degradation,
                             sizeof(degradation));
}

static void
redundancy_manager_publish_telemetry(const redundancy_manager_t *manager) {
    if (manager == NULL) {
        return;
    }

    uint32_t active_count = 0;

    for (size_t i = 0; i < REDUNDANCY_MANAGER_MAX_FAULTS; i++) {
        if (manager->faults[i].active) {
            active_count++;
        }
    }

    // build degraded components bitmask
    uint32_t degraded_mask = 0;

    for (size_t i = 0; i < COMPONENT_COUNT; i++) {
        if (!manager->component_status[i]) {
            degraded_mask |= (1U << i);
        }
    }

    redundancy_telemetry_t telemetry = {.health = manager->health,
                                        .active_fault_count = active_count,
                                        .total_faults_since_boot =
                                            manager->total_fault_count,
                                        .degraded_components = degraded_mask,
                                        .timestamp_ms = hal_time_get_ms()};

    osusat_event_bus_publish(REDUNDANCY_EVENT_TELEMETRY, &telemetry,
                             sizeof(telemetry));
}
