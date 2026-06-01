#include "hal_gpio.h"
#include "hal_i2c.h"
#include "mocks/hal_gpio_mock.h"
#include "mocks/hal_i2c_mock.h"
#include "osusat/event_bus.h"
#include "services/mppt_controller.h"
#include "tests/mocks/event_bus_mock.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define MPPT_UPDATE_INTERVAL_TICKS 10
#define MPPT_ENABLE_PIN 26
#define MPPT_PGOOD_PIN 27

void test_mppt_init(void) {
    printf("Running test: %s\n", __func__);
    mock_event_bus_reset();
    hal_gpio_init();

    mppt_t mppt;
    mppt_init(&mppt);

    assert(mppt.initialized);
    assert(mppt.channels != NULL);
    assert(mppt.num_channels == 1);
    assert(hal_gpio_read(MPPT_ENABLE_PIN) == HAL_GPIO_STATE_LOW);

    printf("Test passed.\n");
}

void test_mppt_enable_disable(void) {
    printf("Running test: %s\n", __func__);
    mock_event_bus_reset();
    hal_gpio_init();

    mppt_t mppt;
    mppt_init(&mppt);

    // enable channel
    mppt_enable(0);
    assert(mppt.channels[0].enabled == true);
    assert(mppt.channels[0].status == MPPT_STATUS_OK);
    assert(hal_gpio_read(MPPT_ENABLE_PIN) == HAL_GPIO_STATE_HIGH);

    // disable channel
    mppt_disable(0);
    assert(mppt.channels[0].enabled == false);
    assert(mppt.channels[0].status == MPPT_STATUS_DISABLED);
    assert(hal_gpio_read(MPPT_ENABLE_PIN) == HAL_GPIO_STATE_LOW);

    printf("Test passed.\n");
}

void test_mppt_pgood_interrupt(void) {
    printf("Running test: %s\n", __func__);
    mock_event_bus_reset();
    hal_gpio_init();

    mppt_t mppt;
    mppt_init(&mppt);

    mppt_enable(0);
    mock_event_bus_reset_published();

    // simulate high to low on pgood pin
    mock_gpio_set_pin_state(MPPT_PGOOD_PIN, HAL_GPIO_STATE_HIGH);

    assert(mppt.channels[0].pgood == true);
    assert(mock_event_bus_get_published_count() == 1);
    captured_event_t event = mock_event_bus_get_published_event(0);
    assert(event.id == MPPT_EVENT_PGOOD_CHANGED);
    assert(event.payload[0] == true);

    printf("Test passed.\n");
}

void test_mppt_telemetry_nominal(void) {
    printf("Running test: %s\n", __func__);
    mock_event_bus_reset();
    hal_gpio_init();

    mppt_t mppt;
    mppt_init(&mppt);
    mppt_enable(0);

    // stage i2c nominal data (9 bytes)
    // status: pgood (0x01)
    // vin: 12000 mV (12.0 V) -> 0x2e, 0xe0
    // iin: 1500 mA (1.5 A) -> 0x05, 0xdc
    // vout: 8000 mV (8.0 V) -> 0x1f, 0x40
    // iout: 2200 mA (2.2 A) -> 0x08, 0x98
    uint8_t telemetry_data[9] = {0x01, 0x2E, 0xE0, 0x05, 0xDC,
                                 0x1F, 0x40, 0x08, 0x98};
    mock_i2c_set_next_read_data(telemetry_data, 9);

    // trigger tick handler to call mppt_perform_update
    for (int i = 0; i < MPPT_UPDATE_INTERVAL_TICKS; i++) {
        mock_event_bus_trigger(EVENT_SYSTICK, NULL, 0);
    }

    assert(mppt.channels[0].pgood == true);
    assert(mppt.channels[0].status == MPPT_STATUS_OK);
    assert(mppt.channels[0].input_voltage > 11.9f &&
           mppt.channels[0].input_voltage < 12.1f);
    assert(mppt.channels[0].input_current > 1.4f &&
           mppt.channels[0].input_current < 1.6f);
    assert(mppt.channels[0].output_voltage > 7.9f &&
           mppt.channels[0].output_voltage < 8.1f);
    assert(mppt.channels[0].output_current > 2.1f &&
           mppt.channels[0].output_current < 2.3f);
    assert(mppt.channels[0].power > 17.5f && mppt.channels[0].power < 17.7f);

    printf("Test passed.\n");
}

void test_mppt_telemetry_fault(void) {
    printf("Running test: %s\n", __func__);
    mock_event_bus_reset();
    hal_gpio_init();

    mppt_t mppt;
    mppt_init(&mppt);
    mppt_enable(0);

    mock_event_bus_reset_published();

    // stage i2c overtemp fault (0x02)
    uint8_t fault_data[9] = {0x02, 0x00, 0x00, 0x00, 0x00,
                             0x00, 0x00, 0x00, 0x00};
    mock_i2c_set_next_read_data(fault_data, 9);

    for (int i = 0; i < MPPT_UPDATE_INTERVAL_TICKS; i++) {
        mock_event_bus_trigger(EVENT_SYSTICK, NULL, 0);
    }

    assert(mppt.channels[0].status == MPPT_STATUS_OVERTEMP);
    assert(mock_event_bus_get_published_count() == 1);
    captured_event_t event = mock_event_bus_get_published_event(0);
    assert(event.id == MPPT_EVENT_FAULT_DETECTED);

    printf("Test passed.\n");
}

int main(void) {
    test_mppt_init();
    test_mppt_enable_disable();
    test_mppt_pgood_interrupt();
    test_mppt_telemetry_nominal();
    test_mppt_telemetry_fault();

    return 0;
}
