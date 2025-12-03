#include "hal/uart.h"
#include "mocks/uart_mock.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

void test_uart_loopback(void) {
    printf("Running test: %s\n", __func__);

    uart_config_t config = {.baudrate = 115200};
    uart_init(UART_PORT_1, &config);

    const char *message = "hello world";
    uart_write(UART_PORT_1, (const uint8_t *)message, strlen(message));

    // the mock doesn't connect tx to rx automatically, so we can't read it back
    // directly. we use mock_uart_get_tx to get what was written.
    uint8_t tx_buf[32];
    size_t tx_len = mock_uart_get_tx(UART_PORT_1, tx_buf, sizeof(tx_buf));

    assert(tx_len == strlen(message));
    assert(memcmp(tx_buf, message, tx_len) == 0);

    printf("Test passed.\n");
}

void test_uart_receive(void) {
    printf("Running test: %s\n", __func__);

    uart_config_t config = {.baudrate = 115200};
    uart_init(UART_PORT_2, &config);

    // push some data into the mock's RX buffer as if it came from outside
    uint8_t rx_data[] = {1, 2, 3, 4, 5};

    for (size_t i = 0; i < sizeof(rx_data); i++) {
        mock_uart_receive_byte_from_isr(UART_PORT_2, rx_data[i]);
    }

    uint8_t read_buf[10];
    uint16_t read_len = uart_read(UART_PORT_2, read_buf, sizeof(read_buf));

    assert(read_len == sizeof(rx_data));
    assert(memcmp(read_buf, rx_data, read_len) == 0);

    // try to read again, should be empty
    read_len = uart_read(UART_PORT_2, read_buf, sizeof(read_buf));
    assert(read_len == 0);

    printf("Test passed.\n");
}

static bool uart_rx_cb_fired = false;

void uart_rx_cb(uart_port_t port, void *ctx) {
    (void)port;
    (void)ctx;
    uart_rx_cb_fired = true;
}

void test_uart_rx_callback(void) {
    printf("Running test: %s\n", __func__);

    uart_config_t config = {.baudrate = 115200};
    uart_init(UART_PORT_3, &config);

    uart_rx_cb_fired = false;
    uart_register_rx_callback(UART_PORT_3, uart_rx_cb, NULL);

    mock_uart_receive_byte_from_isr(UART_PORT_3, 0xAA);
    assert(uart_rx_cb_fired);

    printf("Test passed.\n");
}

int main(void) {
    mock_uart_reset();
    test_uart_loopback();
    test_uart_receive();
    test_uart_rx_callback();

    return 0;
}
