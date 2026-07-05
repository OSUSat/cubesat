
#include "hal_uart_mock.h"
#include "osusat/ring_buffer.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define UART_MOCK_TX_CAPACITY 256

typedef struct {
    bool initialized;

    /* RX handling */
    uart_rx_callback_t rx_cb;
    void *rx_cb_ctx;

    /* Error handling */
    uart_hal_error_cb_t err_cb;
    void *err_cb_ctx;

    uint8_t rx_storage[UART_RX_CAPACITY];
    osusat_ring_buffer_t rx_rb;

    uint8_t tx_storage[UART_MOCK_TX_CAPACITY];
    osusat_ring_buffer_t tx_rb;
} uart_mock_port_t;

static uart_mock_port_t uart_ports[UART_PORT_MAX];

void hal_uart_init(uart_port_t port, const uart_config_t *config) {
    if (port >= UART_PORT_MAX || config == NULL) {
        return;
    }

    uart_mock_port_t *p = &uart_ports[port];
    memset(p, 0, sizeof(*p));

    osusat_ring_buffer_init(&p->rx_rb, p->rx_storage, UART_RX_CAPACITY, false);
    osusat_ring_buffer_init(&p->tx_rb, p->tx_storage, UART_MOCK_TX_CAPACITY,
                            false);

    p->initialized = true;

    printf("MOCK UART%u initialized @ %lu baud\n", port + 1,
           (unsigned long)config->baudrate);
}

void hal_uart_register_rx_callback(uart_port_t port, uart_rx_callback_t cb,
                                   void *ctx) {
    if (port >= UART_PORT_MAX) {
        return;
    }

    uart_ports[port].rx_cb = cb;
    uart_ports[port].rx_cb_ctx = ctx;
}

void hal_uart_register_error_callback(uart_port_t port, uart_hal_error_cb_t cb,
                                      void *ctx) {
    if (port >= UART_PORT_MAX) {
        return;
    }

    uart_ports[port].err_cb = cb;
    uart_ports[port].err_cb_ctx = ctx;
}

void hal_uart_write(uart_port_t port, const uint8_t *data, uint16_t len) {
    if (port >= UART_PORT_MAX || data == NULL || len == 0) {
        return;
    }

    uart_mock_port_t *p = &uart_ports[port];
    if (!p->initialized) {
        return;
    }

    for (uint16_t i = 0; i < len; i++) {
        osusat_ring_buffer_push(&p->tx_rb, data[i]);
    }

    printf("MOCK UART%u wrote %u bytes\n", port + 1, len);
}

uint16_t hal_uart_read(uart_port_t port, uint8_t *out, uint16_t len) {
    if (port >= UART_PORT_MAX || out == NULL || len == 0) {
        return 0;
    }

    uart_mock_port_t *p = &uart_ports[port];
    if (!p->initialized) {
        return 0;
    }

    uint16_t count = 0;
    while (count < len) {
        if (!osusat_ring_buffer_pop(&p->rx_rb, &out[count])) {
            break;
        }
        count++;
    }

    return count;
}

bool mock_uart_receive_byte_from_isr(uart_port_t port, uint8_t byte) {
    if (port >= UART_PORT_MAX) {
        return false;
    }

    uart_mock_port_t *p = &uart_ports[port];
    if (!p->initialized) {
        return false;
    }

    bool ok = osusat_ring_buffer_push(&p->rx_rb, byte);

    if (ok && p->rx_cb != NULL) {
        p->rx_cb(port, p->rx_cb_ctx);
    }

    return ok;
}

bool mock_uart_inject_error(uart_port_t port, uart_error_t err) {
    if (port >= UART_PORT_MAX) {
        return false;
    }

    uart_mock_port_t *p = &uart_ports[port];
    if (!p->initialized || p->err_cb == NULL) {
        return false;
    }

    p->err_cb(port, err, p->err_cb_ctx);
    return true;
}

size_t mock_uart_get_tx(uart_port_t port, uint8_t *out, size_t max_len) {
    if (port >= UART_PORT_MAX || out == NULL || max_len == 0) {
        return 0;
    }

    uart_mock_port_t *p = &uart_ports[port];
    if (!p->initialized) {
        return 0;
    }

    size_t count = 0;
    while (count < max_len) {
        if (!osusat_ring_buffer_pop(&p->tx_rb, &out[count])) {
            break;
        }
        count++;
    }

    return count;
}

void mock_uart_reset(void) {
    for (int i = 0; i < UART_PORT_MAX; i++) {
        osusat_ring_buffer_clear(&uart_ports[i].rx_rb);
        osusat_ring_buffer_clear(&uart_ports[i].tx_rb);
    }
}
