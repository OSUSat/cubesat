/**
 * @file mppt_controller.c
 * @brief MPPT Controller Service Implementation
 */

#include "mppt_controller.h"
#include "eps_config.h"
#include "events.h"
#include "osusat/event_bus.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define MPPT_CONTROLLER_UPDATE_INTEVAL_TICKS 10
#define TELEMETRY_INTERVAL_CYCLES 600

/**
 * @brief System Tick Handler.
 * Called automatically by the Event Bus.
 *
 * @param e   The event (SYSTICK).
 * @param ctx The context pointer (points to mppt_t).
 */
static void mppt_handle_tick(const osusat_event_t *e, void *ctx);

/**
 * @brief Enable Channel Request Handler.
 * Called when the application wants a channel enabled.
 *
 * @param e   The event (SYSTICK).
 * @param ctx The context pointer (points to mppt_t).
 */
static void mppt_handle_request(const osusat_event_t *e, void *ctx);

/**
 * @brief Internal update logic (reads sensors).
 */
static void mppt_perform_update(mppt_t *mppt);

void mppt_init(mppt_t *mppt) {
    if (mppt == NULL) {
        return;
    }

    memset(mppt, 0, sizeof(mppt_t));

    mppt->initialized = true;

    osusat_event_bus_subscribe(EVENT_SYSTICK, mppt_handle_tick, mppt);
    osusat_event_bus_subscribe(APP_EVENT_REQUEST_MPPT_ENABLE_CHANNEL,
                               mppt_handle_request, mppt);
    osusat_event_bus_subscribe(APP_EVENT_REQUEST_MPPT_DISABLE_CHANNEL,
                               mppt_handle_request, mppt);
}

static void mppt_handle_request(const osusat_event_t *e, void *ctx) {
    if (e->payload_len != 1) {
        return;
    }

    if (e->id == APP_EVENT_REQUEST_MPPT_ENABLE_CHANNEL) {
        mppt_enable(e->payload[0]);
    } else {
        mppt_disable(e->payload[0]);
    }

    // manually perform update to process new change
    mppt_perform_update((mppt_t *)ctx);
}

void mppt_enable(uint8_t ch) {
    if (ch >= NUM_MPPT_CHANNELS) {
        return;
    }

    // TODO: enable mppt channel
}

void mppt_disable(uint8_t ch) {
    if (ch >= NUM_MPPT_CHANNELS) {
        return;
    }

    // TODO: disable mppt channel
}

static void mppt_handle_tick(const osusat_event_t *e, void *ctx) {
    (void)e;

    mppt_t *mppt = (mppt_t *)ctx;

    if (mppt == NULL || !mppt->initialized) {
        return;
    }

    mppt->tick_counter++;

    if (mppt->tick_counter >= MPPT_CONTROLLER_UPDATE_INTEVAL_TICKS) {
        mppt->tick_counter = 0;
        mppt_perform_update(mppt);
    }
}

static void mppt_perform_update(mppt_t *mppt) {
    mppt->telemetry_tick_counter++;

    if (mppt->telemetry_tick_counter >= TELEMETRY_INTERVAL_CYCLES) {
        mppt->telemetry_tick_counter = 0;

        // publish telemetry snapshot for every enabled MPPT channel
        for (size_t channel = 0; channel < mppt->num_channels; channel++) {
            if (!mppt->channels[channel].enabled) {
                continue;
            }

            osusat_event_bus_publish(MPPT_EVENT_TELEMETRY, &mppt->channels,
                                     sizeof(mppt_channel_t));
        }
    }
}
