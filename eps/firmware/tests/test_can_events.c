#include "event_bus_mock.h"
#include "hal_can.h"
#include "mocks/hal_can_mock.h"
#include "packet.h"
#include "services/can_events.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

void hal_time_init(void) {}
uint32_t hal_time_get_ms(void) { return 12345; }

void test_can_events_init(void) {
    printf("Running test: %s\n", __func__);
    mock_can_reset();
    mock_event_bus_reset();

    can_events_t can;
    can_events_init(&can, HAL_CAN_PORT_1);

    assert(can.port == HAL_CAN_PORT_1);
    assert(can.initialized);
    assert(can.rx_state == CAN_RX_STATE_WAIT_START_BYTE);
    printf("Test passed.\n");
}

void test_can_events_send_packet(void) {
    printf("Running test: %s\n", __func__);
    mock_can_reset();
    mock_event_bus_reset();

    // initialize CAN port
    hal_can_config_t config = {.baudrate = 250000};
    hal_can_init(HAL_CAN_PORT_1, &config);

    can_events_t can;
    can_events_init(&can, HAL_CAN_PORT_1);

    uint8_t payload[] = "Hello CAN!";
    OSUSatPacket packet = {.version = 1,
                           .destination = OSUSatDestination_OBC,
                           .source = OSUSatDestination_EPS,
                           .message_type = OSUSatMessageType_LOG,
                           .command_id = OSUSatCommonCommand_LOG,
                           .sequence = 0,
                           .is_last_chunk = true,
                           .payload_len = sizeof(payload),
                           .payload = payload};

    can_events_send_packet(&can, &packet);

    // Retrieve transmitted CAN frames from mock
    hal_can_msg_t tx_msgs[10];
    size_t count = mock_can_get_tx(HAL_CAN_PORT_1, tx_msgs, 10);

    // Total size of packed packet:
    // Start (1) + Header (8) + Payload (11) + CRC (2) = 22 bytes
    // 22 bytes divided into 8-byte frames:
    // Frame 1: 8 bytes
    // Frame 2: 8 bytes
    // Frame 3: 6 bytes
    // Total frames = 3
    assert(count == 3);
    assert(tx_msgs[0].dlc == 8);
    assert(tx_msgs[1].dlc == 8);
    assert(tx_msgs[2].dlc == 6);

    // Check CAN ID
    assert(tx_msgs[0].id == CAN_MSG_ID_BASE + OSUSatDestination_OBC);

    // Check event bus published CAN_EVENT_TX_COMPLETE
    int published = mock_event_bus_get_published_count();
    assert(published > 0);
    bool found_tx_complete = false;
    for (int i = 0; i < published; i++) {
        captured_event_t ev = mock_event_bus_get_published_event(i);
        if (ev.id == CAN_EVENT_TX_COMPLETE) {
            found_tx_complete = true;
        }
    }
    assert(found_tx_complete);

    printf("Test passed.\n");
}

void test_can_events_receive_packet(void) {
    printf("Running test: %s\n", __func__);
    mock_can_reset();
    mock_event_bus_reset();

    // initialize CAN port
    hal_can_config_t config = {.baudrate = 250000};
    hal_can_init(HAL_CAN_PORT_1, &config);

    can_events_t can;
    can_events_init(&can, HAL_CAN_PORT_1);

    // Create a packet to serialize and push
    uint8_t payload[] = "Incoming CAN msg!";
    OSUSatPacket packet = {.version = 1,
                           .destination = OSUSatDestination_EPS,
                           .source = OSUSatDestination_OBC,
                           .message_type = OSUSatMessageType_LOG,
                           .command_id = OSUSatCommonCommand_LOG,
                           .sequence = 5,
                           .is_last_chunk = true,
                           .payload_len = sizeof(payload),
                           .payload = payload};

    uint8_t packed_buf[100];
    int16_t packed_len =
        osusat_packet_pack(&packet, packed_buf, sizeof(packed_buf));
    assert(packed_len > 0);

    // push packed_buf to mock RX in 8-byte CAN messages
    uint16_t offset = 0;
    while (offset < packed_len) {
        hal_can_msg_t rx_msg = {
            .id = CAN_MSG_ID_BASE + OSUSatDestination_EPS,
            .id_type = HAL_CAN_ID_STD,
            .rtr = HAL_CAN_RTR_DATA,
            .dlc = (packed_len - offset > 8) ? 8 : (packed_len - offset)};
        memcpy(rx_msg.data, &packed_buf[offset], rx_msg.dlc);
        mock_can_push_rx(HAL_CAN_PORT_1, &rx_msg);
        offset += rx_msg.dlc;
    }

    // Check that CAN_EVENT_PACKET_RECEIVED was published
    int published = mock_event_bus_get_published_count();
    assert(published > 0);
    bool found_packet = false;
    for (int i = 0; i < published; i++) {
        captured_event_t ev = mock_event_bus_get_published_event(i);
        if (ev.id == CAN_EVENT_PACKET_RECEIVED) {
            found_packet = true;
            OSUSatPacket *rx_packet = (OSUSatPacket *)ev.payload;
            assert(rx_packet->version == 1);
            assert(rx_packet->destination == OSUSatDestination_EPS);
            assert(rx_packet->source == OSUSatDestination_OBC);
            assert(rx_packet->sequence == 5);
            assert(rx_packet->payload_len == sizeof(payload));
            assert(memcmp(rx_packet->payload, payload, sizeof(payload)) == 0);
        }
    }
    assert(found_packet);

    printf("Test passed.\n");
}

int main(void) {
    test_can_events_init();
    test_can_events_send_packet();
    test_can_events_receive_packet();
    return 0;
}
