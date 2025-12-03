#include "osusat/event_bus.h"
#include "services/battery_management.h"
#include "tests/mocks/event_bus_mock.h"
#include <assert.h>
#include <stdio.h>

#define BATTERY_UPDATE_INTERVAL_TICKS 10

void test_battery_init(void) {
    printf("Running test: %s\n", __func__);
    mock_event_bus_reset();

    battery_management_t manager;

    battery_init(&manager);

    assert(manager.initialized);
    assert(mock_event_bus_get_published_count() == 1);

    captured_event_t event = mock_event_bus_get_published_event(0);

    assert(event.id == BATTERY_EVENT_SELF_CHECK_PASSED);
    assert(manager.tick_counter == 0);
    assert(manager.telemetry_tick_counter == 0);

    printf("Test passed.\n");
}

void test_battery_critical_low(void) {
    printf("Running test: %s\n", __func__);

    mock_event_bus_reset();

    battery_management_t manager;

    battery_init(&manager);

    mock_event_bus_reset_published(); // clear events from init

    // trigger the tick handler enough times to call battery_perform_update
    for (int i = 0; i < BATTERY_UPDATE_INTERVAL_TICKS; i++) {
        mock_event_bus_trigger(EVENT_SYSTICK, NULL, 0);
    }

    assert(mock_event_bus_get_published_count() > 0);

    bool critical_event_found = false;

    for (int i = 0; i < mock_event_bus_get_published_count(); ++i) {
        captured_event_t event = mock_event_bus_get_published_event(i);

        if (event.id == BATTERY_EVENT_CRITICAL_LOW) {
            critical_event_found = true;
        }
    }

    assert(critical_event_found);
    assert(manager.battery_status.protection);

    printf("Test passed.\n");
}

int main(void) {
    test_battery_init();
    test_battery_critical_low();

    return 0;
}
