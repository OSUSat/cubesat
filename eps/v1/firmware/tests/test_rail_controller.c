/**
 * @file test_rail_controller.c
 * @brief Unit tests for the rail controller service (EPS test plan RC-01 to
 * RC-09 and RC-13). RC-07 (under/overvoltage) is blocked: rail voltage is not
 * measured yet.
 */

#include "event_bus_mock.h"
#include "events.h"
#include "hal_adc_mock.h"
#include "hal_gpio_mock.h"
#include "osusat/event_bus.h"
#include "services/rail_controller.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static rail_controller_t controller;

static void setup_hardware(void) {
    mock_event_bus_reset();
    hal_gpio_init();

    // on hardware the load switch enable pins are configured as outputs by
    // board GPIO init; the mock starts every pin as an input
    for (uint8_t rail = 0; rail < NUM_POWER_RAILS; rail++) {
        hal_gpio_set_mode(RAIL_ENABLE_PIN(rail), HAL_GPIO_MODE_OUTPUT);
        mock_adc_set_value((adc_channel_t)rail, 0);
    }
}

static void setup(void) {
    setup_hardware();
    rail_controller_init(&controller);
}

static void run_update(void) {
    for (int i = 0; i < RAIL_CONTROLLER_UPDATE_INTERVAL_TICKS; i++) {
        mock_event_bus_trigger(EVENT_SYSTICK, NULL, 0);
    }
}

static void request_rail(osusat_event_id_t request, uint8_t rail) {
    mock_event_bus_trigger(request, &rail, sizeof(rail));
}

static int count_published(osusat_event_id_t id) {
    int count = 0;

    for (int i = 0; i < mock_event_bus_get_published_count(); i++) {
        if (mock_event_bus_get_published_event(i).id == id) {
            count++;
        }
    }

    return count;
}

static void assert_all_rails_off(void) {
    for (uint8_t rail = 0; rail < NUM_POWER_RAILS; rail++) {
        assert(!controller.rails[rail].enabled);
        assert(hal_gpio_read(RAIL_ENABLE_PIN(rail)) == HAL_GPIO_STATE_LOW);
    }
}

// RC-01
void test_init_drives_all_rails_off(void) {
    printf("Running test: %s\n", __func__);
    setup_hardware();

    // start with every load switch on, as after a service re-init
    for (uint8_t rail = 0; rail < NUM_POWER_RAILS; rail++) {
        hal_gpio_write(RAIL_ENABLE_PIN(rail), HAL_GPIO_STATE_HIGH);
    }

    rail_controller_init(&controller);

    assert(controller.initialized);
    assert_all_rails_off();

    for (uint8_t rail = 0; rail < NUM_POWER_RAILS; rail++) {
        assert(controller.rails[rail].status == RAIL_STATUS_DISABLED);
    }

    printf("Test passed.\n");
}

// RC-02
void test_enable_disable_requests(void) {
    printf("Running test: %s\n", __func__);
    setup();

    request_rail(APP_EVENT_REQUEST_RAIL_CONTROLLER_ENABLE_RAIL, RAIL_RADIO);

    assert(controller.rails[RAIL_RADIO].enabled);
    assert(controller.rails[RAIL_RADIO].status == RAIL_STATUS_OK);
    assert(hal_gpio_read(RAIL_ENABLE_PIN(RAIL_RADIO)) == HAL_GPIO_STATE_HIGH);

    request_rail(APP_EVENT_REQUEST_RAIL_CONTROLLER_DISABLE_RAIL, RAIL_RADIO);

    assert(!controller.rails[RAIL_RADIO].enabled);
    assert(controller.rails[RAIL_RADIO].status == RAIL_STATUS_DISABLED);
    assert(hal_gpio_read(RAIL_ENABLE_PIN(RAIL_RADIO)) == HAL_GPIO_STATE_LOW);

    printf("Test passed.\n");
}

// RC-03
void test_invalid_requests_ignored(void) {
    printf("Running test: %s\n", __func__);
    setup();

    // out-of-range rail
    request_rail(APP_EVENT_REQUEST_RAIL_CONTROLLER_ENABLE_RAIL,
                 NUM_POWER_RAILS);

    // wrong payload length
    uint8_t bad_payload[2] = {RAIL_RADIO, 0};
    mock_event_bus_trigger(APP_EVENT_REQUEST_RAIL_CONTROLLER_ENABLE_RAIL,
                           bad_payload, sizeof(bad_payload));

    assert_all_rails_off();

    printf("Test passed.\n");
}

// RC-06
void test_below_limit_is_not_fault(void) {
    printf("Running test: %s\n", __func__);
    setup();

    request_rail(APP_EVENT_REQUEST_RAIL_CONTROLLER_ENABLE_RAIL, RAIL_RADIO);
    mock_event_bus_reset_published();

    // full scale maps to 120% of the limit, so 3000 counts is ~88%
    mock_adc_set_value((adc_channel_t)RAIL_RADIO, 3000);
    run_update();

    assert(controller.rails[RAIL_RADIO].enabled);
    assert(controller.rails[RAIL_RADIO].status == RAIL_STATUS_OK);
    assert(count_published(RAIL_CONTROLLER_EVENT_OVERCURRENT_DETECTED) == 0);

    printf("Test passed.\n");
}

// RC-08
void test_disabled_rail_never_faults(void) {
    printf("Running test: %s\n", __func__);
    setup();

    mock_adc_set_value((adc_channel_t)RAIL_RADIO, 4095);
    run_update();

    assert(controller.rails[RAIL_RADIO].status == RAIL_STATUS_DISABLED);
    assert(count_published(RAIL_CONTROLLER_EVENT_OVERCURRENT_DETECTED) == 0);

    printf("Test passed.\n");
}

// RC-04
void test_overcurrent_shuts_off_rail(void) {
    printf("Running test: %s\n", __func__);
    setup();

    request_rail(APP_EVENT_REQUEST_RAIL_CONTROLLER_ENABLE_RAIL, RAIL_OBC);
    request_rail(APP_EVENT_REQUEST_RAIL_CONTROLLER_ENABLE_RAIL, RAIL_RADIO);
    mock_event_bus_reset_published();

    mock_adc_set_value((adc_channel_t)RAIL_RADIO, 4095);
    run_update();

    assert(controller.rails[RAIL_RADIO].status == RAIL_STATUS_OVERCURRENT);
    assert(!controller.rails[RAIL_RADIO].enabled);
    assert(hal_gpio_read(RAIL_ENABLE_PIN(RAIL_RADIO)) == HAL_GPIO_STATE_LOW);

    // payload is a rail_t snapshot taken after the shutoff
    assert(count_published(RAIL_CONTROLLER_EVENT_OVERCURRENT_DETECTED) == 1);
    captured_event_t event = mock_event_bus_get_published_event(0);
    assert(event.payload_len == sizeof(rail_t));

    rail_t snapshot;
    memcpy(&snapshot, event.payload, sizeof(snapshot));
    assert(snapshot.rail_id == RAIL_RADIO);
    assert(snapshot.status == RAIL_STATUS_OVERCURRENT);
    assert(!snapshot.enabled);

    // other rails are untouched
    assert(controller.rails[RAIL_OBC].enabled);
    assert(hal_gpio_read(RAIL_ENABLE_PIN(RAIL_OBC)) == HAL_GPIO_STATE_HIGH);

    printf("Test passed.\n");
}

// RC-05
void test_fault_reported_once_per_trip(void) {
    printf("Running test: %s\n", __func__);
    setup();

    request_rail(APP_EVENT_REQUEST_RAIL_CONTROLLER_ENABLE_RAIL, RAIL_RADIO);
    mock_event_bus_reset_published();
    mock_adc_set_value((adc_channel_t)RAIL_RADIO, 4095);

    // a tripped rail stays off and is not reported again
    for (int cycle = 0; cycle < 5; cycle++) {
        run_update();
    }

    assert(count_published(RAIL_CONTROLLER_EVENT_OVERCURRENT_DETECTED) == 1);
    assert(hal_gpio_read(RAIL_ENABLE_PIN(RAIL_RADIO)) == HAL_GPIO_STATE_LOW);

    // re-enabling into the same fault trips and reports it again
    request_rail(APP_EVENT_REQUEST_RAIL_CONTROLLER_ENABLE_RAIL, RAIL_RADIO);
    run_update();

    assert(count_published(RAIL_CONTROLLER_EVENT_OVERCURRENT_DETECTED) == 2);
    assert(hal_gpio_read(RAIL_ENABLE_PIN(RAIL_RADIO)) == HAL_GPIO_STATE_LOW);

    printf("Test passed.\n");
}

// RC-13
void test_fault_retried_when_queue_full(void) {
    printf("Running test: %s\n", __func__);
    setup();

    request_rail(APP_EVENT_REQUEST_RAIL_CONTROLLER_ENABLE_RAIL, RAIL_RADIO);
    mock_event_bus_reset_published();

    // fill the mock queue so the fault publish fails
    while (osusat_event_bus_publish(EVENT_SYSTICK, NULL, 0)) {
    }

    mock_adc_set_value((adc_channel_t)RAIL_RADIO, 4095);
    run_update();

    // the rail is shut off even though the event could not be sent
    assert(hal_gpio_read(RAIL_ENABLE_PIN(RAIL_RADIO)) == HAL_GPIO_STATE_LOW);
    assert(count_published(RAIL_CONTROLLER_EVENT_OVERCURRENT_DETECTED) == 0);

    // simulate the queue draining; the next update delivers the fault once
    mock_event_bus_reset_published();
    run_update();
    run_update();

    assert(count_published(RAIL_CONTROLLER_EVENT_OVERCURRENT_DETECTED) == 1);

    rail_t snapshot;
    memcpy(&snapshot, mock_event_bus_get_published_event(0).payload,
           sizeof(snapshot));
    assert(snapshot.rail_id == RAIL_RADIO);
    assert(snapshot.status == RAIL_STATUS_OVERCURRENT);

    printf("Test passed.\n");
}

// RC-09
void test_telemetry_published_on_interval(void) {
    printf("Running test: %s\n", __func__);
    setup();

    for (int i = 0; i < RAIL_CONTROLLER_TELEMETRY_INTERVAL_TICKS; i++) {
        mock_event_bus_trigger(EVENT_SYSTICK, NULL, 0);
    }

    assert(count_published(RAIL_CONTROLLER_EVENT_TELEMETRY) == NUM_POWER_RAILS);

    printf("Test passed.\n");
}

int main(void) {
    test_init_drives_all_rails_off();
    test_enable_disable_requests();
    test_invalid_requests_ignored();
    test_below_limit_is_not_fault();
    test_disabled_rail_never_faults();
    test_overcurrent_shuts_off_rail();
    test_fault_reported_once_per_trip();
    test_fault_retried_when_queue_full();
    test_telemetry_published_on_interval();

    return 0;
}
