#include "event_bus_mock.h"
#include "events.h"
#include "hal_gpio.h"
#include "hal_gpio_mock.h"
#include "osusat/event_bus.h"
#include "rail_controller.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

static void trigger_update(void) {
    for (int i = 0; i < 10; i++) {
        mock_event_bus_trigger(EVENT_SYSTICK, NULL, 0);
    }
}

void test_rail_controller_init(void) {
    printf("Running test: %s\n", __func__);
    osusat_event_bus_init(NULL, 0);
    hal_gpio_init();

    rail_controller_t manager;
    rail_controller_init(&manager);

    assert(manager.initialized == true);
    for (size_t rail = 0; rail < NUM_POWER_RAILS; rail++) {
        assert(manager.rails[rail].rail_id == (power_rail_t)rail);
        assert(manager.rails[rail].enabled == false);
        assert(manager.rails[rail].status == RAIL_STATUS_DISABLED);
        assert(manager.rails[rail].voltage == 0.0f);
        assert(manager.rails[rail].current == 0.0f);
    }
    printf("Test passed.\n");
}

void test_rail_controller_enable_disable(void) {
    printf("Running test: %s\n", __func__);
    osusat_event_bus_init(NULL, 0);
    hal_gpio_init();

    rail_controller_t manager;
    rail_controller_init(&manager);

    // enable rail gps
    rail_controller_enable(&manager, RAIL_GPS);
    assert(manager.rails[RAIL_GPS].enabled == true);
    assert(manager.rails[RAIL_GPS].status == RAIL_STATUS_OK);
    assert(hal_gpio_read(8 + RAIL_GPS) == HAL_GPIO_STATE_HIGH);

    // disable rail gps
    rail_controller_disable(&manager, RAIL_GPS);
    assert(manager.rails[RAIL_GPS].enabled == false);
    assert(manager.rails[RAIL_GPS].status == RAIL_STATUS_DISABLED);
    assert(hal_gpio_read(8 + RAIL_GPS) == HAL_GPIO_STATE_LOW);

    printf("Test passed.\n");
}

void test_rail_controller_handle_update_nominal(void) {
    printf("Running test: %s\n", __func__);
    osusat_event_bus_init(NULL, 0);
    hal_gpio_init();

    rail_controller_t manager;
    rail_controller_init(&manager);

    // enable rail obc
    rail_controller_enable(&manager, RAIL_OBC);

    // run update via event bus tick
    trigger_update();

    // should fall back to sensible defaults and remain ok
    assert(manager.rails[RAIL_OBC].enabled == true);
    assert(manager.rails[RAIL_OBC].status == RAIL_STATUS_OK);
    assert(manager.rails[RAIL_OBC].voltage == 3.3f);
    assert(manager.rails[RAIL_OBC].current == 0.1f);

    printf("Test passed.\n");
}

void test_rail_controller_handle_update_faults(void) {
    printf("Running test: %s\n", __func__);
    osusat_event_bus_init(NULL, 0);
    hal_gpio_init();

    rail_controller_t manager;
    rail_controller_init(&manager);

    // 1. overcurrent fault
    rail_controller_enable(&manager, RAIL_RADIO);
    trigger_update();
    assert(manager.rails[RAIL_RADIO].enabled == true);

    // inject high current
    manager.rails[RAIL_RADIO].current = 3.0f; // limit is 1.5f
    trigger_update();

    assert(manager.rails[RAIL_RADIO].enabled == false);
    assert(manager.rails[RAIL_RADIO].status == RAIL_STATUS_OVERCURRENT);
    assert(hal_gpio_read(8 + RAIL_RADIO) == HAL_GPIO_STATE_LOW);

    // 2. undervoltage fault
    rail_controller_enable(&manager, RAIL_GPS);
    trigger_update();
    assert(manager.rails[RAIL_GPS].enabled == true);

    // inject low voltage
    manager.rails[RAIL_GPS].voltage = 2.5f; // limit min is 3.0f
    trigger_update();

    assert(manager.rails[RAIL_GPS].enabled == false);
    assert(manager.rails[RAIL_GPS].status == RAIL_STATUS_UNDERVOLTAGE);
    assert(hal_gpio_read(8 + RAIL_GPS) == HAL_GPIO_STATE_LOW);

    // 3. overvoltage fault
    rail_controller_enable(&manager, RAIL_5V_BUS);
    trigger_update();
    assert(manager.rails[RAIL_5V_BUS].enabled == true);

    // inject high voltage
    manager.rails[RAIL_5V_BUS].voltage = 6.0f; // limit max is 5.25f
    trigger_update();

    assert(manager.rails[RAIL_5V_BUS].enabled == false);
    assert(manager.rails[RAIL_5V_BUS].status == RAIL_STATUS_OVERVOLTAGE);
    assert(hal_gpio_read(8 + RAIL_5V_BUS) == HAL_GPIO_STATE_LOW);

    printf("Test passed.\n");
}

int main(void) {
    test_rail_controller_init();
    test_rail_controller_enable_disable();
    test_rail_controller_handle_update_nominal();
    test_rail_controller_handle_update_faults();

    return 0;
}
