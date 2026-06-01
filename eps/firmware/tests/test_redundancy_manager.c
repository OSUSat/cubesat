#include "event_bus_mock.h"
#include "events.h"
#include "mppt_controller.h"
#include "osusat/event_bus.h"
#include "rail_controller.h"
#include "redundancy_manager.h"
#include "uart_events.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

void test_redundancy_init(void) {
    printf("Running test: %s\n", __func__);
    mock_event_bus_reset();

    redundancy_manager_t manager;
    redundancy_manager_init(&manager);

    assert(manager.initialized);
    assert(manager.health == SYSTEM_HEALTH_OK);

    // Verify all components initialized to OK (true)
    for (size_t i = 0; i < COMPONENT_COUNT; i++) {
        assert(manager.component_status[i] == true);
    }

    printf("Test passed.\n");
}

void test_redundancy_mppt_fault(void) {
    printf("Running test: %s\n", __func__);
    mock_event_bus_reset();

    redundancy_manager_t manager;
    redundancy_manager_init(&manager);

    // Trigger an MPPT fault for channel 2 (which should map to
    // COMPONENT_SOLAR_STRING_3)
    uint8_t failed_channel = 2;
    mock_event_bus_trigger(MPPT_EVENT_FAULT_DETECTED, &failed_channel,
                           sizeof(uint8_t));

    // Verify solar string 3 is now degraded, but others are nominal
    assert(manager.component_status[COMPONENT_SOLAR_STRING_3] == false);
    assert(manager.component_status[COMPONENT_SOLAR_STRING_1] == true);
    assert(manager.component_status[COMPONENT_SOLAR_STRING_2] == true);
    assert(manager.component_status[COMPONENT_SOLAR_STRING_4] == true);

    // Verify system health degraded
    assert(manager.health == SYSTEM_HEALTH_DEGRADED);

    printf("Test passed.\n");
}

void test_redundancy_uart_fault(void) {
    printf("Running test: %s\n", __func__);
    mock_event_bus_reset();

    redundancy_manager_t manager;
    redundancy_manager_init(&manager);

    // Trigger a UART error on port 1 (COMPONENT_UART_PRIMARY)
    uint8_t payload[2] = {1, 0xAA}; // Port 1, some error code
    mock_event_bus_trigger(UART_EVENT_ERROR_DETECTED, payload, sizeof(payload));

    // Verify component status updated
    assert(manager.component_status[COMPONENT_UART_PRIMARY] == false);
    assert(manager.component_status[COMPONENT_UART_SECONDARY] == true);

    printf("Test passed.\n");
}

void test_redundancy_component_status_query(void) {
    printf("Running test: %s\n", __func__);
    mock_event_bus_reset();

    redundancy_manager_t manager;
    redundancy_manager_init(&manager);

    // Degrade COMPONENT_UART_SECONDARY (port 3)
    uint8_t payload[2] = {3, 0xBB}; // Port 3, error code
    mock_event_bus_trigger(UART_EVENT_ERROR_DETECTED, payload, sizeof(payload));

    assert(manager.component_status[COMPONENT_UART_SECONDARY] == false);

    // Reset published event mock before querying
    mock_event_bus_reset_published();

    // Request status for COMPONENT_UART_SECONDARY
    component_status_request_t req = {.component = COMPONENT_UART_SECONDARY};
    mock_event_bus_trigger(APP_EVENT_REQUEST_REDUNDANCY_COMPONENT_STATUS, &req,
                           sizeof(req));

    // Verify response was published
    assert(mock_event_bus_get_published_count() == 1);
    captured_event_t event = mock_event_bus_get_published_event(0);
    assert(event.id == REDUNDANCY_EVENT_COMPONENT_STATUS_RESPONSE);

    component_status_response_t response;
    memcpy(&response, event.payload, sizeof(component_status_response_t));

    assert(response.component == COMPONENT_UART_SECONDARY);
    assert(response.is_ok == false);
    assert(response.fault_source == FAULT_SOURCE_UART);

    printf("Test passed.\n");
}

void test_redundancy_clear_fault(void) {
    printf("Running test: %s\n", __func__);
    mock_event_bus_reset();

    redundancy_manager_t manager;
    redundancy_manager_init(&manager);

    // Degrade COMPONENT_UART_PRIMARY
    uint8_t payload[2] = {1, 0xCC};
    mock_event_bus_trigger(UART_EVENT_ERROR_DETECTED, payload, sizeof(payload));
    assert(manager.component_status[COMPONENT_UART_PRIMARY] == false);

    // Extract the recorded fault to clear it
    fault_t fault_to_clear = {
        .source = FAULT_SOURCE_UART,
        .code = (1 << 8) | OSUSAT_GET_LOCAL_CODE(UART_EVENT_ERROR_DETECTED),
    };

    mock_event_bus_trigger(APP_EVENT_REQUEST_REDUNDANCY_CLEAR_FAULT,
                           &fault_to_clear, sizeof(fault_to_clear));

    // Verify component status recovers to true (OK)
    assert(manager.component_status[COMPONENT_UART_PRIMARY] == true);
    assert(manager.health == SYSTEM_HEALTH_OK);

    printf("Test passed.\n");
}

int main(void) {
    test_redundancy_init();
    test_redundancy_mppt_fault();
    test_redundancy_uart_fault();
    test_redundancy_component_status_query();
    test_redundancy_clear_fault();

    return 0;
}
