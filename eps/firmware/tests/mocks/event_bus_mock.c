#include "tests/mocks/event_bus_mock.h"
#include <stdio.h>
#include <string.h>

#define MAX_SUBSCRIBERS 16
#define MAX_PUBLISHED_EVENTS 16

typedef struct {
    osusat_event_id_t id;
    osusat_event_handler_t handler;
    void *ctx;
} subscription_t;

static subscription_t subscriptions[MAX_SUBSCRIBERS];
static int subscriber_count = 0;

static captured_event_t published_events[MAX_PUBLISHED_EVENTS];
static int published_count = 0;

void osusat_event_bus_init(osusat_event_t *queue_storage,
                           size_t queue_capacity) {
    (void)queue_storage;
    (void)queue_capacity;

    mock_event_bus_reset();
}

bool osusat_event_bus_subscribe(osusat_event_id_t event_id,
                                osusat_event_handler_t handler, void *ctx) {
    if (subscriber_count < MAX_SUBSCRIBERS) {
        subscriptions[subscriber_count++] =
            (subscription_t){event_id, handler, ctx};

        return true;
    }

    return false;
}

bool osusat_event_bus_publish(osusat_event_id_t event_id, const void *payload,
                              size_t len) {
    if (published_count < MAX_PUBLISHED_EVENTS) {
        captured_event_t *e = &published_events[published_count++];

        e->id = event_id;
        e->payload_len = len;

        if (payload && len > 0) {
            memcpy(e->payload, payload, len);
        }

        return true;
    }
    return false;
}

void osusat_event_bus_process(void) {
    // in this mock, events are processed immediately by mock_event_bus_trigger
}

// mock-specific functions
int mock_event_bus_get_published_count(void) { return published_count; }

captured_event_t mock_event_bus_get_published_event(int index) {
    return published_events[index];
}

void mock_event_bus_reset_published(void) {
    published_count = 0;
    memset(published_events, 0, sizeof(published_events));
}

void mock_event_bus_reset_subscribers(void) {
    subscriber_count = 0;
    memset(subscriptions, 0, sizeof(subscriptions));
}

void mock_event_bus_reset(void) {
    mock_event_bus_reset_published();
    mock_event_bus_reset_subscribers();
}

void mock_event_bus_trigger(osusat_event_id_t event_id, const void *payload,
                            size_t len) {
    for (int i = 0; i < subscriber_count; i++) {
        if (subscriptions[i].id == event_id) {
            osusat_event_t event;

            event.id = event_id;
            event.payload_len = len;

            if (payload && len > 0) {
                memcpy(event.payload, payload, len);
            }

            subscriptions[i].handler(&event, subscriptions[i].ctx);
        }
    }
}
