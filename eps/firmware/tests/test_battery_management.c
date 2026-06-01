#include "hal_adc_mock.h"
#include "hal_gpio.h"
#include "osusat/event_bus.h"
#include "services/battery_management.h"
#include "tests/mocks/event_bus_mock.h"
#include <assert.h>
#include <stdio.h>

#define BATTERY_UPDATE_INTERVAL_TICKS 10

void test_battery_init(void) {
    printf("Running test: %s\n", __func__);
    mock_event_bus_reset();
    hal_gpio_init();
    hal_adc_init();

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
    hal_gpio_init();
    hal_adc_init();

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

void test_battery_charge_control(void) {
    printf("Running test: %s\n", __func__);

    mock_event_bus_reset();
    hal_gpio_init();
    hal_adc_init();

    battery_management_t manager;
    battery_init(&manager);

    mock_event_bus_reset_published(); // clear events from init

    // 1. Enable charge control
    battery_charge_control(&manager, true);

    assert(manager.battery_status.charging == true);
    assert(manager.battery_status.balancing == true);

    // Verify GPIO states using hal_gpio_read
    assert(hal_gpio_read(24) == HAL_GPIO_STATE_HIGH);
    assert(hal_gpio_read(25) == HAL_GPIO_STATE_HIGH);

    assert(mock_event_bus_get_published_count() == 1);
    captured_event_t event = mock_event_bus_get_published_event(0);
    assert(event.id == BATTERY_EVENT_CHARGING_CHANGE);
    assert(event.payload[0] == true);

    // 2. Disable charge control
    mock_event_bus_reset_published();
    battery_charge_control(&manager, false);

    assert(manager.battery_status.charging == false);
    assert(manager.battery_status.balancing == false);

    assert(hal_gpio_read(24) == HAL_GPIO_STATE_LOW);
    assert(hal_gpio_read(25) == HAL_GPIO_STATE_LOW);

    assert(mock_event_bus_get_published_count() == 1);
    event = mock_event_bus_get_published_event(0);
    assert(event.id == BATTERY_EVENT_CHARGING_CHANGE);
    assert(event.payload[0] == false);

    printf("Test passed.\n");
}

void test_battery_diagnostics(void) {
    printf("Running test: %s\n", __func__);

    mock_event_bus_reset();
    hal_gpio_init();
    hal_adc_init();

    // 1. Nominal case (raw ADC value > 0 and healthy)
    mock_adc_set_value(ADC_CHANNEL_0, 3500); // ~3.84V

    battery_management_t manager;
    battery_init(&manager);

    assert(manager.initialized == true);
    captured_event_t event = mock_event_bus_get_published_event(0);
    assert(event.id == BATTERY_EVENT_SELF_CHECK_PASSED);

    // 2. Unhealthy initial voltage case (e.g. non-zero but below threshold)
    mock_event_bus_reset();
    mock_adc_set_value(ADC_CHANNEL_0, 2000); // 2.2V (< 3.3V)

    battery_init(&manager);

    assert(manager.initialized == false);
    event = mock_event_bus_get_published_event(0);
    assert(event.id == BATTERY_EVENT_SELF_CHECK_FAILED);
    assert(event.payload[0] == 0x01);

    printf("Test passed.\n");
}

int main(void) {
    test_battery_init();
    test_battery_critical_low();
    test_battery_charge_control();
    test_battery_diagnostics();

    return 0;
}
