#include "event_bus_mock.h"
#include "mocks/hal_can_mock.h"
#include "osusat/slog.h"
#include "packet.h"
#include "services/can_events.h"
#include "services/logging.h"
#include "hal_flash.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static uint32_t current_time_ms = 0;
uint32_t hal_time_get_ms(void) { return current_time_ms; }

void test_logging_init(void) {
    printf("Running test: %s\n", __func__);
    mock_can_reset();
    mock_event_bus_reset();

    // initialize CAN ports
    hal_can_config_t config = {.baudrate = 250000};
    hal_can_init(HAL_CAN_PORT_1, &config);
    hal_can_init(HAL_CAN_PORT_2, &config);

    can_events_t primary_can;
    can_events_t aux_can;
    can_events_init(&primary_can, HAL_CAN_PORT_1);
    can_events_init(&aux_can, HAL_CAN_PORT_2);

    logging_init(OSUSAT_SLOG_INFO, &primary_can, &aux_can);

    assert(logging_pending_count() == 1);
    logging_flush();
    assert(logging_pending_count() == 0);
    printf("Test passed.\n");
}

void test_logging_flush(void) {
    printf("Running test: %s\n", __func__);
    mock_can_reset();
    mock_event_bus_reset();

    // initialize CAN port
    hal_can_config_t config = {.baudrate = 250000};
    hal_can_init(HAL_CAN_PORT_1, &config);
    hal_can_init(HAL_CAN_PORT_2, &config);

    can_events_t primary_can;
    can_events_t aux_can;
    can_events_init(&primary_can, HAL_CAN_PORT_1);
    can_events_init(&aux_can, HAL_CAN_PORT_2);

    logging_init(OSUSAT_SLOG_INFO, &primary_can, &aux_can);

    // flush the initialization log message first
    logging_flush();
    assert(logging_pending_count() == 0);

    // log something long enough to exceed 40 bytes
    LOG_INFO(EPS_COMPONENT_MAIN,
             "Test log message! This is a long message to ensure that the "
             "pending count is greater than zero.");
    printf("Pending count: %zu\n", logging_pending_count());
    assert(logging_pending_count() >= 1);

    // flush
    size_t count = logging_flush();
    assert(count == 1);
    assert(logging_pending_count() == 0);

    // Retrieve transmitted CAN frames from mock
    hal_can_msg_t tx_msgs[10];
    size_t tx_count = mock_can_get_tx(HAL_CAN_PORT_1, tx_msgs, 10);
    assert(tx_count > 0);

    printf("Test passed.\n");
}

void test_logging_flash_backup(void) {
    printf("Running test: %s\n", __func__);
    mock_can_reset();
    mock_event_bus_reset();

    // initialize CAN port
    hal_can_config_t config = {.baudrate = 250000};
    hal_can_init(HAL_CAN_PORT_1, &config);
    hal_can_init(HAL_CAN_PORT_2, &config);

    can_events_t primary_can;
    can_events_t aux_can;
    can_events_init(&primary_can, HAL_CAN_PORT_1);
    can_events_init(&aux_can, HAL_CAN_PORT_2);

    logging_init(OSUSAT_SLOG_INFO, &primary_can, &aux_can);

    // flush the initialization log message first
    size_t count1 = logging_flush();
    assert(count1 == 1);

    // log something long enough
    LOG_INFO(EPS_COMPONENT_MAIN,
             "Test log message! This is a long message to ensure that the "
             "pending count is greater than zero.");

    // flush
    size_t count2 = logging_flush();
    assert(count2 == 1); // 1 test log

    // Read back mock flash
    uint8_t flash_buf[512];
    hal_flash_read(FLASH_LOG_START_ADDR, flash_buf, sizeof(flash_buf));

    OSUSatPacket first_packet;
    OSUSatPacketResult res1 =
        osusat_packet_unpack(&first_packet, flash_buf, sizeof(flash_buf));
    assert(res1 == OSUSAT_PACKET_OK);

    size_t first_packet_size = OSUSAT_FRAME_OVERHEAD + first_packet.payload_len;

    OSUSatPacket second_packet;
    OSUSatPacketResult res2 =
        osusat_packet_unpack(&second_packet, flash_buf + first_packet_size,
                             sizeof(flash_buf) - first_packet_size);
    assert(res2 == OSUSAT_PACKET_OK);
    assert(second_packet.version == 1);
    assert(second_packet.destination == OSUSatDestination_OBC);
    assert(second_packet.source == OSUSatDestination_EPS);
    assert(second_packet.message_type == OSUSatMessageType_LOG);
    assert(second_packet.command_id == OSUSatCommonCommand_LOG);

    printf("Test passed.\n");
}

int main(void) {
    test_logging_init();
    test_logging_flush();
    test_logging_flash_backup();
    return 0;
}
