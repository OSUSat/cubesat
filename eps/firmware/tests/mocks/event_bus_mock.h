#ifndef EVENT_BUS_MOCK_H
#define EVENT_BUS_MOCK_H

#include "osusat/event_bus.h"

// struct to hold a captured event
typedef struct {
    osusat_event_id_t id;
    uint8_t payload[OSUSAT_EVENT_MAX_PAYLOAD];
    size_t payload_len;
} captured_event_t;

// get the number of published events
int mock_event_bus_get_published_count(void);

// get a specific published event
captured_event_t mock_event_bus_get_published_event(int index);

// reset the mock's published event history
void mock_event_bus_reset_published(void);

// reset the mock's subscribers
void mock_event_bus_reset_subscribers(void);

// reset the entire mock state
void mock_event_bus_reset(void);

// manually trigger a subscribed handler
void mock_event_bus_trigger(osusat_event_id_t event_id, const void *payload,
                            size_t len);

#endif
