#include "hal_can.h"
#include "hal_can_mock.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static bool tx_callback_fired = false;
static bool rx_callback_fired = false;
static hal_can_msg_t last_rx_msg;

void can_tx_cb(hal_can_port_t port, void *ctx) {
    (void)port;
    (void)ctx;
    tx_callback_fired = true;
}

void can_rx_cb(hal_can_port_t port, const hal_can_msg_t *msg, void *ctx) {
    (void)port;
    (void)ctx;
    rx_callback_fired = true;
    last_rx_msg = *msg;
}

void test_can_init_and_transmit(void) {
    printf("Running test: %s\n", __func__);

    hal_can_config_t config = {.baudrate = 250000};
    hal_can_init(HAL_CAN_PORT_1, &config);

    tx_callback_fired = false;
    hal_can_register_tx_callback(HAL_CAN_PORT_1, can_tx_cb, NULL);

    hal_can_msg_t tx_msg = {
        .id = 0x123,
        .id_type = HAL_CAN_ID_STD,
        .rtr = HAL_CAN_RTR_DATA,
        .dlc = 4,
        .data = {0xAA, 0xBB, 0xCC, 0xDD}
    };

    hal_can_status_t status = hal_can_write(HAL_CAN_PORT_1, &tx_msg);
    assert(status == HAL_CAN_OK);
    assert(tx_callback_fired);

    hal_can_msg_t retrieved_msg = {0};
    size_t count = mock_can_get_tx(HAL_CAN_PORT_1, &retrieved_msg, 1);
    assert(count == 1);
    assert(retrieved_msg.id == tx_msg.id);
    assert(retrieved_msg.id_type == tx_msg.id_type);
    assert(retrieved_msg.rtr == tx_msg.rtr);
    assert(retrieved_msg.dlc == tx_msg.dlc);
    assert(memcmp(retrieved_msg.data, tx_msg.data, 4) == 0);

    printf("Test passed.\n");
}

void test_can_receive(void) {
    printf("Running test: %s\n", __func__);

    hal_can_config_t config = {.baudrate = 500000};
    hal_can_init(HAL_CAN_PORT_2, &config);

    rx_callback_fired = false;
    hal_can_register_rx_callback(HAL_CAN_PORT_2, can_rx_cb, NULL);

    hal_can_msg_t rx_msg = {
        .id = 0x1FABCDE,
        .id_type = HAL_CAN_ID_EXT,
        .rtr = HAL_CAN_RTR_DATA,
        .dlc = 8,
        .data = {1, 2, 3, 4, 5, 6, 7, 8}
    };

    mock_can_push_rx(HAL_CAN_PORT_2, &rx_msg);
    assert(rx_callback_fired);
    assert(last_rx_msg.id == rx_msg.id);
    assert(last_rx_msg.id_type == rx_msg.id_type);
    assert(last_rx_msg.rtr == rx_msg.rtr);
    assert(last_rx_msg.dlc == rx_msg.dlc);
    assert(memcmp(last_rx_msg.data, rx_msg.data, 8) == 0);

    printf("Test passed.\n");
}

int main(void) {
    mock_can_reset();
    test_can_init_and_transmit();
    test_can_receive();
    return 0;
}
