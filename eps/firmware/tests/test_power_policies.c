#include "app/power_policies.h"
#include "event_bus_mock.h"
#include "events.h"
#include "mppt_controller.h"
#include "osusat/event_bus.h"
#include "redundancy_manager.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

void test_mppt_fault_cooldown(void) {
    printf("Running test: %s\n", __func__);
    mock_event_bus_reset();

    power_policies_t app;
    power_policies_init(&app);

    // trigger mppt fault for channel 2
    uint8_t failed_channel = 2;
    mock_event_bus_trigger(MPPT_EVENT_FAULT_DETECTED, &failed_channel,
                           sizeof(uint8_t));

    // verify disable channel event was published
    assert(mock_event_bus_get_published_count() > 0);
    bool disabled_published = false;
    for (int i = 0; i < mock_event_bus_get_published_count(); i++) {
        captured_event_t event = mock_event_bus_get_published_event(i);
        if (event.id == APP_EVENT_REQUEST_MPPT_DISABLE_CHANNEL) {
            assert(event.payload[0] == failed_channel);
            disabled_published = true;
        }
    }
    assert(disabled_published);

    mock_event_bus_reset_published();

    // simulate 499 ticks (under 500 tick cooldown)
    for (int i = 0; i < 499; i++) {
        mock_event_bus_trigger(EVENT_SYSTICK, NULL, 0);
    }

    // verify no enable event has been published yet
    for (int i = 0; i < mock_event_bus_get_published_count(); i++) {
        captured_event_t event = mock_event_bus_get_published_event(i);
        assert(event.id != APP_EVENT_REQUEST_MPPT_ENABLE_CHANNEL);
    }

    // trigger the final tick to complete cooldown
    mock_event_bus_trigger(EVENT_SYSTICK, NULL, 0);

    // verify enable event was published for channel 2
    bool enabled_published = false;
    for (int i = 0; i < mock_event_bus_get_published_count(); i++) {
        captured_event_t event = mock_event_bus_get_published_event(i);
        if (event.id == APP_EVENT_REQUEST_MPPT_ENABLE_CHANNEL) {
            assert(event.payload[0] == failed_channel);
            enabled_published = true;
        }
    }
    assert(enabled_published);

    printf("Test passed.\n");
}

void test_redundancy_health_changes(void) {
    printf("Running test: %s\n", __func__);
    mock_event_bus_reset();

    power_policies_t app;
    power_policies_init(&app);

    mock_event_bus_reset_published();

    // trigger critical health fault
    system_health_t health = SYSTEM_HEALTH_FAULT;
    mock_event_bus_trigger(REDUNDANCY_EVENT_SYSTEM_HEALTH_CHANGED, &health,
                           sizeof(system_health_t));

    // verify request to transition to safe mode
    bool safe_published = false;
    for (int i = 0; i < mock_event_bus_get_published_count(); i++) {
        captured_event_t event = mock_event_bus_get_published_event(i);
        if (event.id == APP_EVENT_REQUEST_POWER_PROFILE_SAFE) {
            safe_published = true;
        }
    }
    assert(safe_published);

    mock_event_bus_reset_published();

    // trigger health recovery
    health = SYSTEM_HEALTH_OK;
    mock_event_bus_trigger(REDUNDANCY_EVENT_SYSTEM_HEALTH_CHANGED, &health,
                           sizeof(system_health_t));

    // verify request to transition back to nominal mode
    bool nominal_published = false;
    for (int i = 0; i < mock_event_bus_get_published_count(); i++) {
        captured_event_t event = mock_event_bus_get_published_event(i);
        if (event.id == APP_EVENT_REQUEST_POWER_PROFILE_NOMINAL) {
            nominal_published = true;
        }
    }
    assert(nominal_published);

    printf("Test passed.\n");
}

int main(void) {
    test_mppt_fault_cooldown();
    test_redundancy_health_changes();

    return 0;
}
