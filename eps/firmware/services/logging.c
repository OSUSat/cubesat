/**
 * @file logging.c
 * @brief EPS Logging Service Implementation
 */

#include "logging.h"
#include "events.h"
#include "hal_time.h"
#include "hal_uart.h"
#include "messages.h"
#include "osusat/event_bus.h"
#include "osusat/ring_buffer.h"
#include "osusat/slog.h"
#include "packet.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define LOG_FLUSH_INTERVAL_CYCLES 600

#define LOG_STORAGE_SIZE 4096
static uint8_t log_storage[LOG_STORAGE_SIZE];
static osusat_ring_buffer_t log_ring_buffer;
static uint32_t tick_counter;

#define LOG_PACKET_MAX_PAYLOAD 200

typedef struct {
    uint16_t sequence;
    uint8_t payload_buffer[LOG_PACKET_MAX_PAYLOAD];
    size_t payload_offset; // write position in the payload buffer
} log_flush_context_t;

/**
 * @brief System Tick Handler.
 * Called automatically by the Event Bus.
 *
 * @param e   The event (SYSTICK).
 * @param ctx The context pointer.
 */
static void logging_handle_tick(const osusat_event_t *e, void *ctx);

/**
 * @brief Event Handler.
 * Called by the Event Bus.
 *
 * @param e   The event (SYSTICK).
 * @param ctx The context pointer.
 */
static void logging_handle_request(const osusat_event_t *e, void *ctx);

/**
 * @brief Helper to send OSUSatPacket objects over UART to the OBC.
 *
 * @param log_flush_context_t   The log flush context.
 * @param is_last Whether this is the last packet in the sequence.
 */
static void send_log_packet(log_flush_context_t *ctx, bool is_last);

static void send_log_packet(log_flush_context_t *ctx, bool is_last) {
    if (ctx->payload_offset == 0) {
        return;
    }

    OSUSatPacket packet = {.version = 1,
                           .destination = OSUSatDestination_OBC,
                           .source = OSUSatDestination_EPS,
                           .message_type = OSUSatMessageType_LOG,
                           .command_id = OSUSatCommonCommand_LOG,
                           .sequence = ctx->sequence,
                           .is_last_chunk = is_last,
                           .payload_len = (uint8_t)ctx->payload_offset,
                           .payload = ctx->payload_buffer};

    uint8_t tx_buffer[256];
    int16_t packed_size =
        osusat_packet_pack(&packet, tx_buffer, sizeof(tx_buffer));

    if (packed_size > 0) {
        // TODO: handle service degredations from redundancy manager
        // it should alert all affected services to use AUX UART port
        // for now just use main
        hal_uart_write(UART_PORT_1, tx_buffer, (uint16_t)packed_size);
    }

    ctx->payload_offset = 0; // reset cursor for next packet
}

/**
 * @brief Flush callback - packs log entries into packets
 *
 * Multiple log entries may be batched into a single packet.
 * When a log entry won't fit, the current packet is sent and a new one is
 * started.
 */
static void log_flush_callback(const osusat_slog_entry_t *entry,
                               const char *message, void *user_ctx) {
    log_flush_context_t *ctx = (log_flush_context_t *)user_ctx;

    // calculate size of this complete log entry (header + message + null
    // terminator)
    size_t entry_size = sizeof(*entry) + entry->message_len + 1;

    // send current packet first if entry is too big
    if (ctx->payload_offset + entry_size > LOG_PACKET_MAX_PAYLOAD) {
        send_log_packet(ctx, false);
        ctx->sequence++;
    }

    memcpy(ctx->payload_buffer + ctx->payload_offset, entry, sizeof(*entry));
    ctx->payload_offset += sizeof(*entry);

    memcpy(ctx->payload_buffer + ctx->payload_offset, message,
           entry->message_len + 1);
    ctx->payload_offset += entry->message_len + 1;
}

void logging_init(osusat_slog_level_t min_level) {
    osusat_ring_buffer_init(&log_ring_buffer, log_storage, sizeof(log_storage),
                            true);

    osusat_slog_init(&log_ring_buffer, hal_time_get_ms, min_level);

    osusat_event_bus_subscribe(EVENT_SYSTICK, logging_handle_tick, NULL);
    osusat_event_bus_subscribe(APP_EVENT_REQUEST_LOGGING_FLUSH_LOGS,
                               logging_handle_request, NULL);

    LOG_INFO(EPS_COMPONENT_MAIN, "Logging service initialized");
}

static void logging_handle_tick(const osusat_event_t *e, void *ctx) {
    (void)ctx;
    (void)e;

    tick_counter++;

    if (tick_counter >= LOG_FLUSH_INTERVAL_CYCLES) {
        tick_counter = 0;
        logging_flush();
    }
}

static void logging_handle_request(const osusat_event_t *e, void *ctx) {
    (void)ctx;
    (void)e;

    logging_flush();
}

size_t logging_flush(void) {
    log_flush_context_t ctx = {.sequence = 0, .payload_offset = 0};

    size_t count = osusat_slog_flush(log_flush_callback, &ctx);

    if (ctx.payload_offset > 0) {
        send_log_packet(&ctx, true);
    }

    return count;
}

void logging_set_level(osusat_slog_level_t level) {
    osusat_slog_change_min_log_level(level);
    LOG_INFO(EPS_COMPONENT_MAIN, "Log level changed to %d", level);
}

size_t logging_pending_count(void) { return osusat_slog_pending_count(); }
