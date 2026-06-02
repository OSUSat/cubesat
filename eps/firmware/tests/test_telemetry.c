/**
 * @file test_telemetry.c
 * @brief Unit tests for the telemetry aggregation service.
 */

#include "services/telemetry.h"
#include "hal_time.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static uint32_t simulated_time_ms = 12345;

void hal_time_init(void) {
    // no-op for tests
}

uint32_t hal_time_get_ms(void) {
    return simulated_time_ms;
}

void test_telemetry_init(void) {
    printf("Running test: %s\n", __func__);

    telemetry_t telemetry;
    telemetry_init(&telemetry);

    // check that the structure is completely zeroed out
    assert(telemetry.battery_manager == NULL);
    assert(telemetry.mppt_controller == NULL);
    assert(telemetry.rail_controller == NULL);
    assert(telemetry.redundancy_manager == NULL);
    assert(telemetry.usart1_events == NULL);
    assert(telemetry.usart3_events == NULL);

    assert(telemetry.telemetry.battery.voltage == 0.0f);
    assert(telemetry.telemetry.battery.current == 0.0f);
    assert(telemetry.telemetry.battery.charging == false);

    for (size_t i = 0; i < NUM_MPPT_CHANNELS; i++) {
        assert(telemetry.telemetry.mppt_channels[i].input_voltage == 0.0f);
        assert(telemetry.telemetry.mppt_channels[i].enabled == false);
    }

    for (size_t i = 0; i < NUM_POWER_RAILS; i++) {
        assert(telemetry.telemetry.rails[i].voltage == 0.0f);
        assert(telemetry.telemetry.rails[i].enabled == false);
    }

    assert(telemetry.telemetry.redundancy.health == SYSTEM_HEALTH_OK);
    assert(telemetry.telemetry.redundancy.active_fault_count == 0);

    assert(telemetry.telemetry.uart1.rx_byte_count == 0);
    assert(telemetry.telemetry.uart3.rx_byte_count == 0);

    printf("Test passed.\n");
}

void test_telemetry_update(void) {
    printf("Running test: %s\n", __func__);

    telemetry_t telemetry;
    telemetry_init(&telemetry);

    // mock instances of other services
    battery_management_t battery;
    memset(&battery, 0, sizeof(battery));
    battery.battery_status.voltage = 3.8f;
    battery.battery_status.current = -0.5f;
    battery.battery_status.temperature = 25.0f;
    battery.battery_status.soc = 85.0f;
    battery.battery_status.charging = false;
    battery.battery_status.protection = false;

    mppt_t mppt;
    memset(&mppt, 0, sizeof(mppt));
    mppt_channel_t mppt_chan;
    memset(&mppt_chan, 0, sizeof(mppt_chan));
    mppt_chan.input_voltage = 5.2f;
    mppt_chan.input_current = 0.8f;
    mppt_chan.output_voltage = 4.2f;
    mppt_chan.output_current = 0.95f;
    mppt_chan.power = 3.99f;
    mppt_chan.enabled = true;
    mppt_chan.pgood = true;
    mppt.channels = &mppt_chan;
    mppt.num_channels = 1;

    rail_controller_t rail_ctrl;
    memset(&rail_ctrl, 0, sizeof(rail_ctrl));
    rail_ctrl.rails[RAIL_OBC].rail_id = RAIL_OBC;
    rail_ctrl.rails[RAIL_OBC].voltage = 3.31f;
    rail_ctrl.rails[RAIL_OBC].current = 0.45f;
    rail_ctrl.rails[RAIL_OBC].enabled = true;
    rail_ctrl.rails[RAIL_OBC].status = RAIL_STATUS_OK;

    rail_ctrl.rails[RAIL_RADIO].rail_id = RAIL_RADIO;
    rail_ctrl.rails[RAIL_RADIO].voltage = 5.02f;
    rail_ctrl.rails[RAIL_RADIO].current = 0.12f;
    rail_ctrl.rails[RAIL_RADIO].enabled = false;
    rail_ctrl.rails[RAIL_RADIO].status = RAIL_STATUS_DISABLED;

    redundancy_manager_t redundancy;
    memset(&redundancy, 0, sizeof(redundancy));
    redundancy.health = SYSTEM_HEALTH_DEGRADED;
    redundancy.total_fault_count = 3;
    redundancy.faults[0].active = true;
    redundancy.faults[1].active = false;
    redundancy.faults[2].active = true;
    for (size_t i = 0; i < COMPONENT_COUNT; i++) {
        redundancy.component_status[i] = true;
    }
    redundancy.component_status[COMPONENT_UART_PRIMARY] = false; // degraded
    redundancy.component_status[COMPONENT_UART_SECONDARY] = true;

    uart_events_t uart1;
    memset(&uart1, 0, sizeof(uart1));
    uart1.rx_byte_count = 1024;
    uart1.rx_packet_count = 42;
    uart1.rx_crc_error_count = 2;
    uart1.initialized = true;

    uart_events_t uart3;
    memset(&uart3, 0, sizeof(uart3));
    uart3.rx_byte_count = 512;
    uart3.rx_packet_count = 20;
    uart3.rx_crc_error_count = 0;
    uart3.initialized = true;

    // set service pointers in the telemetry manager
    telemetry.battery_manager = &battery;
    telemetry.mppt_controller = &mppt;
    telemetry.rail_controller = &rail_ctrl;
    telemetry.redundancy_manager = &redundancy;
    telemetry.usart1_events = &uart1;
    telemetry.usart3_events = &uart3;

    simulated_time_ms = 98765;

    // execute telemetry update
    telemetry_update(&telemetry);

    // retrieve master packet
    eps_telemetry_t tlm = telemetry_get_all(&telemetry);

    // verify battery metrics
    assert(tlm.battery.voltage == 3.8f);
    assert(tlm.battery.current == -0.5f);
    assert(tlm.battery.charging == false);

    // verify mppt metrics
    assert(tlm.mppt_channels[0].input_voltage == 5.2f);
    assert(tlm.mppt_channels[0].power == 3.99f);
    assert(tlm.mppt_channels[0].enabled == true);

    // verify power rails metrics
    assert(tlm.rails[RAIL_OBC].voltage == 3.31f);
    assert(tlm.rails[RAIL_OBC].enabled == true);
    assert(tlm.rails[RAIL_RADIO].voltage == 5.02f);
    assert(tlm.rails[RAIL_RADIO].enabled == false);

    // verify redundancy metrics
    assert(tlm.redundancy.health == SYSTEM_HEALTH_DEGRADED);
    assert(tlm.redundancy.total_faults_since_boot == 3);
    assert(tlm.redundancy.active_fault_count == 2);
    assert(tlm.redundancy.degraded_components == (1U << COMPONENT_UART_PRIMARY));
    assert(tlm.redundancy.timestamp_ms == 98765);

    // verify uart metrics
    assert(tlm.uart1.rx_byte_count == 1024);
    assert(tlm.uart1.rx_crc_error_count == 2);
    assert(tlm.uart1.initialized == true);

    assert(tlm.uart3.rx_byte_count == 512);
    assert(tlm.uart3.rx_crc_error_count == 0);
    assert(tlm.uart3.initialized == true);

    printf("Test passed.\n");
}

void test_telemetry_null_pointers(void) {
    printf("Running test: %s\n", __func__);

    telemetry_t telemetry;
    telemetry_init(&telemetry);

    // with all pointers NULL, update should not crash and should remain zero
    telemetry_update(&telemetry);

    eps_telemetry_t tlm = telemetry_get_all(&telemetry);
    assert(tlm.battery.voltage == 0.0f);
    assert(tlm.redundancy.health == SYSTEM_HEALTH_OK);
    assert(tlm.uart1.rx_byte_count == 0);

    // telemetry_update should handle NULL input pointer gracefully
    telemetry_update(NULL);

    printf("Test passed.\n");
}

int main(void) {
    hal_time_init();
    test_telemetry_init();
    test_telemetry_update();
    test_telemetry_null_pointers();

    return 0;
}
