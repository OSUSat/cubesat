/**
 * @file logging.c
 * @brief EPS Logging Service Implementation
 */

#include "logging.h"
#include "can_events.h"
#include "events.h"
#include "hal_time.h"
#include "messages.h"
#include "osusat/event_bus.h"
#include "osusat/ring_buffer.h"
#include "osusat/slog.h"
#include "packet.h"
#include "redundancy_manager.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define MAX_LOG_ENTRIES_PER_FLUSH 5
#define LOG_FLUSH_INTERVAL_CYCLES 600

#define LOG_STORAGE_SIZE 4096
static uint8_t log_storage[LOG_STORAGE_SIZE];
static osusat_ring_buffer_t log_ring_buffer;
static uint32_t tick_counter;

// pointer to the CAN service instance we use for flushing
static can_events_t *g_primary_can = NULL;
static can_events_t *g_aux_can = NULL;
static can_events_t *g_active_can = NULL;

#define LOG_PACKET_MAX_PAYLOAD 200

typedef struct {
    uint16_t sequence;
    uint8_t payload_buffer[LOG_PACKET_MAX_PAYLOAD];
    size_t payload_offset;
    int entries_processed;
} log_flush_context_t;

static void logging_handle_tick(const osusat_event_t *e, void *ctx);
static void logging_handle_request(const osusat_event_t *e, void *ctx);
static void logging_handle_redundancy(const osusat_event_t *e, void *ctx);
static void send_log_packet(log_flush_context_t *ctx, bool is_last);

void logging_init(osusat_slog_level_t min_level, can_events_t *primary_can,
                  can_events_t *aux_can) {
    g_primary_can = primary_can;
    g_aux_can = aux_can;

    g_active_can = g_primary_can;

    osusat_ring_buffer_init(&log_ring_buffer, log_storage, sizeof(log_storage),
                            true);

    osusat_slog_init(&log_ring_buffer, hal_time_get_ms, min_level);

    osusat_event_bus_subscribe(EVENT_SYSTICK, logging_handle_tick, NULL);
    osusat_event_bus_subscribe(APP_EVENT_REQUEST_LOGGING_FLUSH_LOGS,
                               logging_handle_request, NULL);

    osusat_event_bus_subscribe(REDUNDANCY_EVENT_COMPONENT_DEGRADED,
                               logging_handle_redundancy, NULL);
    osusat_event_bus_subscribe(REDUNDANCY_EVENT_COMPONENT_RECOVERED,
                               logging_handle_redundancy, NULL);

    LOG_INFO(EPS_COMPONENT_MAIN,
             "Logging service initialized (Primary: CAN%d, Aux: CAN%d)",
             primary_can->port + 1, aux_can->port + 1);
}

static void send_log_packet(log_flush_context_t *ctx, bool is_last) {
    // don't flush if we have no CAN service connected
    if (ctx->payload_offset == 0 || g_active_can == NULL ||
        !g_active_can->initialized) {
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

    can_events_send_packet(g_active_can, &packet);

    ctx->payload_offset = 0;
}

static void log_flush_callback(const osusat_slog_entry_t *entry,
                               const char *message, void *user_ctx) {
    if (((log_flush_context_t *)user_ctx)->entries_processed >=
        MAX_LOG_ENTRIES_PER_FLUSH) {
        return;
    }

    log_flush_context_t *ctx = (log_flush_context_t *)user_ctx;

    size_t entry_size = sizeof(*entry) + entry->message_len + 1;

    if (ctx->payload_offset + entry_size > LOG_PACKET_MAX_PAYLOAD) {
        send_log_packet(ctx, false);
        ctx->sequence++;
    }

    memcpy(ctx->payload_buffer + ctx->payload_offset, entry, sizeof(*entry));
    ctx->payload_offset += sizeof(*entry);

    memcpy(ctx->payload_buffer + ctx->payload_offset, message,
           entry->message_len + 1);
    ctx->payload_offset += entry->message_len + 1;

    ctx->entries_processed++;
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
    if (g_active_can == NULL || !g_active_can->initialized) {
        return 0;
    }

    log_flush_context_t ctx = {.sequence = 0, .payload_offset = 0};

    if (osusat_slog_pending_count() == 0) {
        return 0;
    }

    size_t count = osusat_slog_flush(log_flush_callback, &ctx);

    if (ctx.payload_offset > 0) {
        bool is_last = (osusat_slog_pending_count() == 0);
        send_log_packet(&ctx, is_last);
    }

    return count;
}

void logging_set_level(osusat_slog_level_t level) {
    osusat_slog_change_min_log_level(level);
    LOG_INFO(EPS_COMPONENT_MAIN, "Log level changed to %d", level);
}

size_t logging_pending_count(void) { return osusat_slog_pending_count(); }

/**
 * @brief Handles failover events from the Redundancy Manager
 */
static void logging_handle_redundancy(const osusat_event_t *e, void *ctx) {
    (void)ctx;

    if (e->id == REDUNDANCY_EVENT_COMPONENT_DEGRADED) {
        component_degradation_t *payload =
            (component_degradation_t *)e->payload;

        // if primary CAN failed, switch to AUX
        if (payload->component == COMPONENT_CAN_PRIMARY) {
            if (g_aux_can && g_aux_can->initialized) {
                g_active_can = g_aux_can;
            }
        }
    } else if (e->id == REDUNDANCY_EVENT_COMPONENT_RECOVERED) {
        if (e->payload_len >= sizeof(component_id_t)) {
            component_id_t comp = *(component_id_t *)e->payload;

            if (comp == COMPONENT_CAN_PRIMARY) {
                g_active_can = g_primary_can;
            }
        }
    }
}
