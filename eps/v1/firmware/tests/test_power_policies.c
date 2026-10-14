/**
 * @file test_power_policies.c
 * @brief Unit tests for the power policies application (EPS test plan PP-01 to
 * PP-03).
 */

#include "battery_management.h"
#include "event_bus_mock.h"
#include "events.h"
#include "mppt_controller.h"
#include "osusat/event_bus.h"
#include "power_policies.h"
#include <assert.h>
#include <stdio.h>

static power_policies_t policies;

static void setup(void) {
    mock_event_bus_reset();
    power_policies_init(&policies);
}

// PP-01
void test_critical_battery_requests_safe_mode(void) {
    printf("Running test: %s\n", __func__);
    setup();

    mock_event_bus_trigger(BATTERY_EVENT_CRITICAL_LOW, NULL, 0);

    assert(mock_event_bus_get_published_count() == 1);
    assert(mock_event_bus_get_published_event(0).id ==
           APP_EVENT_REQUEST_POWER_PROFILE_SAFE);

    printf("Test passed.\n");
}

// PP-02
void test_fully_charged_requests_nominal_mode(void) {
    printf("Running test: %s\n", __func__);
    setup();

    mock_event_bus_trigger(BATTERY_EVENT_FULLY_CHARGED, NULL, 0);

    assert(mock_event_bus_get_published_count() == 1);
    assert(mock_event_bus_get_published_event(0).id ==
           APP_EVENT_REQUEST_POWER_PROFILE_NOMINAL);

    printf("Test passed.\n");
}

// PP-03
void test_mppt_fault_disables_that_channel(void) {
    printf("Running test: %s\n", __func__);
    setup();

    uint8_t channel = 3;
    mock_event_bus_trigger(MPPT_EVENT_FAULT_DETECTED, &channel, sizeof(channel));

    assert(mock_event_bus_get_published_count() == 1);

    captured_event_t event = mock_event_bus_get_published_event(0);
    assert(event.id == APP_EVENT_REQUEST_MPPT_DISABLE_CHANNEL);
    assert(event.payload_len == sizeof(uint8_t));
    assert(event.payload[0] == channel);

    printf("Test passed.\n");
}

// PP-03
void test_mppt_fault_without_channel_ignored(void) {
    printf("Running test: %s\n", __func__);
    setup();

    mock_event_bus_trigger(MPPT_EVENT_FAULT_DETECTED, NULL, 0);

    assert(mock_event_bus_get_published_count() == 0);

    printf("Test passed.\n");
}

int main(void) {
    test_critical_battery_requests_safe_mode();
    test_fully_charged_requests_nominal_mode();
    test_mppt_fault_disables_that_channel();
    test_mppt_fault_without_channel_ignored();

    return 0;
}
