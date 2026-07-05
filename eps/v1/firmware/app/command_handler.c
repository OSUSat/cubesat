/**
 * @file command_handler.c
 * @brief Application layer for processing incoming commands.
 */

#include "command_handler.h"
#include "events.h"
#include "messages.h"
#include "osusat/event_bus.h"
#include "packet.h"
#include <string.h>

static void handle_uart_event(const osusat_event_t *e, void *ctx)
    __attribute__((unused));
static void process_packet(command_handler_t *app, const OSUSatPacket *packet)
    __attribute__((unused));

void command_handler_init(command_handler_t *app) {
    if (app == NULL) {
        return;
    }

    memset(app, 0, sizeof(command_handler_t));
    app->initialized = true;

    // TODO: subscribe to uart events
    // osusat_event_bus_subscribe(UART_EVENT_PACKET_RECEIVED, handle_uart_event,
    // app);
}

static void handle_uart_event(const osusat_event_t *e, void *ctx) {
    command_handler_t *app __attribute__((unused)) = (command_handler_t *)ctx;

    // if (e->id == UART_EVENT_PACKET_RECEIVED) {
    //     OSUSatPacket *packet = (OSUSatPacket *)e->payload;
    //     process_packet(app, packet);
    // }
}

static void process_packet(command_handler_t *app, const OSUSatPacket *packet) {
    // TODO: implement command processing logic.
    //
    // switch (packet->message_id) {
    // case MSG_ID_TOGGLE_SAFE_MODE:
    // {
    //      // request a change to safe mode over the event bus
    //      osusat_event_bus_publish(APP_EVENT_REQUEST_POWER_PROFILE_SAFE, NULL,
    //      0);
    // }
    // default:
    //     break;
    // }
}
