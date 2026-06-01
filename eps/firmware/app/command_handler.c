/**
 * @file command_handler.c
 * @brief Application layer for processing incoming commands.
 */

#include "command_handler.h"
#include "events.h"
#include "logging.h"
#include "messages.h"
#include "osusat/event_bus.h"
#include "osusat/slog.h"
#include "packet.h"
#include "redundancy_manager.h"
#include <string.h>

static void handle_uart_event(const osusat_event_t *e, void *ctx);
static void process_packet(command_handler_t *app, const OSUSatPacket *packet);
static void handle_redundancy(const osusat_event_t *e, void *ctx);
static void handle_health_response(const osusat_event_t *e, void *ctx);
static void send_reply(command_handler_t *app, const OSUSatPacket *incoming,
                       OSUSatMessageType type, const uint8_t *payload,
                       uint8_t len);

void command_handler_init(command_handler_t *app, uart_events_t *primary_uart,
                          uart_events_t *aux_uart) {
    if (app == NULL) {
        return;
    }

    memset(app, 0, sizeof(command_handler_t));
    app->primary_uart = primary_uart;
    app->aux_uart = aux_uart;
    app->primary_uart_ok = true;
    app->initialized = true;

    // register event bus subscriptions
    osusat_event_bus_subscribe(UART_EVENT_PACKET_RECEIVED, handle_uart_event,
                               app);
    osusat_event_bus_subscribe(REDUNDANCY_EVENT_COMPONENT_DEGRADED,
                               handle_redundancy, app);
    osusat_event_bus_subscribe(REDUNDANCY_EVENT_COMPONENT_RECOVERED,
                               handle_redundancy, app);
    osusat_event_bus_subscribe(REDUNDANCY_EVENT_HEALTH_RESPONSE,
                               handle_health_response, app);
}

static void handle_uart_event(const osusat_event_t *e, void *ctx) {
    command_handler_t *app = (command_handler_t *)ctx;
    if (app == NULL) {
        return;
    }

    if (e->id == UART_EVENT_PACKET_RECEIVED) {
        const OSUSatPacket *packet = (const OSUSatPacket *)e->payload;
        process_packet(app, packet);
    }
}

static void handle_redundancy(const osusat_event_t *e, void *ctx) {
    command_handler_t *app = (command_handler_t *)ctx;
    if (app == NULL) {
        return;
    }

    if (e->id == REDUNDANCY_EVENT_COMPONENT_DEGRADED) {
        if (e->payload_len >= sizeof(component_degradation_t)) {
            const component_degradation_t *payload =
                (const component_degradation_t *)e->payload;
            if (payload->component == COMPONENT_UART_PRIMARY) {
                app->primary_uart_ok = false;
            }
        }
    } else if (e->id == REDUNDANCY_EVENT_COMPONENT_RECOVERED) {
        if (e->payload_len >= sizeof(component_id_t)) {
            component_id_t comp = *(const component_id_t *)e->payload;
            if (comp == COMPONENT_UART_PRIMARY) {
                app->primary_uart_ok = true;
            }
        }
    }
}

static void send_reply(command_handler_t *app, const OSUSatPacket *incoming,
                       OSUSatMessageType type, const uint8_t *payload,
                       uint8_t len) {
    OSUSatPacket reply = {.version = 1,
                          .destination = incoming->source,
                          .source = OSUSatDestination_EPS,
                          .message_type = type,
                          .command_id = incoming->command_id,
                          .sequence = incoming->sequence,
                          .is_last_chunk = true,
                          .payload_len = len,
                          .payload = (uint8_t *)payload};

    // check if primary uart is ok, otherwise fallback to aux
    uart_events_t *target_uart = (app->primary_uart_ok && app->primary_uart)
                                     ? app->primary_uart
                                     : app->aux_uart;

    if (target_uart && target_uart->initialized) {
        uart_events_send_packet(target_uart, &reply);
    }
}

static void process_packet(command_handler_t *app, const OSUSatPacket *packet) {
    if (app == NULL || packet == NULL) {
        return;
    }

    // validate incoming command packet
    if (packet->destination != OSUSatDestination_EPS ||
        packet->message_type != OSUSatMessageType_COMMAND) {
        return;
    }

    switch (packet->command_id) {
    case OSUSatCommonCommand_HEARTBEAT:
        // heartbeat is always successful, reply with ack
        send_reply(app, packet, OSUSatMessageType_ACK, NULL, 0);
        break;

    case OSUSatCommonCommand_HEALTHCHECK:
        // healthcheck is always successful, reply with ack
        send_reply(app, packet, OSUSatMessageType_ACK, NULL, 0);
        break;

    case OSUSatEPSCommand_ENABLE_RAIL:
        if (packet->payload_len >= 1) {
            uint8_t rail_id = packet->payload[0];
            osusat_event_bus_publish(
                APP_EVENT_REQUEST_RAIL_CONTROLLER_ENABLE_RAIL, &rail_id, 1);
            send_reply(app, packet, OSUSatMessageType_ACK, NULL, 0);
        } else {
            send_reply(app, packet, OSUSatMessageType_NACK, NULL, 0);
        }
        break;

    case OSUSatEPSCommand_DISABLE_RAIL:
        if (packet->payload_len >= 1) {
            uint8_t rail_id = packet->payload[0];
            osusat_event_bus_publish(
                APP_EVENT_REQUEST_RAIL_CONTROLLER_DISABLE_RAIL, &rail_id, 1);
            send_reply(app, packet, OSUSatMessageType_ACK, NULL, 0);
        } else {
            send_reply(app, packet, OSUSatMessageType_NACK, NULL, 0);
        }
        break;

    case OSUSatGSECommand_TOGGLE_SAFE_MODE: {
        // payload specifies 1 for safe mode, 0 for nominal mode
        bool enable_safe = (packet->payload_len > 0 && packet->payload[0] != 0);
        if (enable_safe) {
            osusat_event_bus_publish(APP_EVENT_REQUEST_POWER_PROFILE_SAFE, NULL,
                                     0);
        } else {
            osusat_event_bus_publish(APP_EVENT_REQUEST_POWER_PROFILE_NOMINAL,
                                     NULL, 0);
        }
        send_reply(app, packet, OSUSatMessageType_ACK, NULL, 0);
        break;
    }

    case OSUSatGSECommand_PROBE_SUBSYSTEM_STATUS:
        // store the context to map the asynchronous health response back to
        // this command
        app->pending_reply = true;
        app->pending_sequence = packet->sequence;
        app->pending_destination = packet->source;
        app->pending_command_id = packet->command_id;

        // publish request to retrieve subsystem health
        osusat_event_bus_publish(APP_EVENT_REQUEST_REDUNDANCY_HEALTH, NULL, 0);
        break;

    default:
        // unsupported command id, reply with nack
        send_reply(app, packet, OSUSatMessageType_NACK, NULL, 0);
        break;
    }
}

static void handle_health_response(const osusat_event_t *e, void *ctx) {
    command_handler_t *app = (command_handler_t *)ctx;
    if (app == NULL || !app->pending_reply) {
        return;
    }

    if (e->id == REDUNDANCY_EVENT_HEALTH_RESPONSE) {
        // construct the reply packet manually based on stored context
        OSUSatPacket reply = {.version = 1,
                              .destination = app->pending_destination,
                              .source = OSUSatDestination_EPS,
                              .message_type = OSUSatMessageType_TELEMETRY,
                              .command_id = app->pending_command_id,
                              .sequence = app->pending_sequence,
                              .is_last_chunk = true,
                              .payload_len = (uint8_t)e->payload_len,
                              .payload = (uint8_t *)e->payload};

        uart_events_t *target_uart = (app->primary_uart_ok && app->primary_uart)
                                         ? app->primary_uart
                                         : app->aux_uart;

        if (target_uart && target_uart->initialized) {
            uart_events_send_packet(target_uart, &reply);
        }

        // clear the pending reply context
        app->pending_reply = false;
    }
}
