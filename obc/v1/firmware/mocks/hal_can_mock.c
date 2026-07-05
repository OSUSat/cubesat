#include "hal_can_mock.h"
#include <stdio.h>
#include <string.h>

#define MOCK_CAN_QUEUE_SIZE 32

typedef struct {
    bool initialized;
    hal_can_rx_cb_t rx_cb;
    void *rx_cb_ctx;
    hal_can_tx_cb_t tx_cb;
    void *tx_cb_ctx;
    hal_can_error_cb_t err_cb;
    void *err_cb_ctx;

    hal_can_msg_t tx_queue[MOCK_CAN_QUEUE_SIZE];
    size_t tx_head;
    size_t tx_tail;
    size_t tx_count;
} can_mock_port_t;

static can_mock_port_t can_ports[HAL_CAN_PORT_MAX];

void hal_can_init(hal_can_port_t port, const hal_can_config_t *config) {
    if (port >= HAL_CAN_PORT_MAX || config == NULL) {
        return;
    }

    can_mock_port_t *p = &can_ports[port];
    memset(p, 0, sizeof(*p));
    p->initialized = true;

    printf("MOCK CAN%u initialized @ %lu baud\n", port + 1,
           (unsigned long)config->baudrate);
}

hal_can_status_t hal_can_write(hal_can_port_t port, const hal_can_msg_t *msg) {
    if (port >= HAL_CAN_PORT_MAX || msg == NULL) {
        return HAL_CAN_ERROR;
    }

    can_mock_port_t *p = &can_ports[port];
    if (!p->initialized) {
        return HAL_CAN_ERROR;
    }

    if (p->tx_count >= MOCK_CAN_QUEUE_SIZE) {
        return HAL_CAN_BUSY;
    }

    /* store transmitted message in mock tx queue */
    p->tx_queue[p->tx_head] = *msg;
    p->tx_head = (p->tx_head + 1) % MOCK_CAN_QUEUE_SIZE;
    p->tx_count++;

    /* fire TX complete callback to simulate non-blocking transmission */
    if (p->tx_cb != NULL) {
        p->tx_cb(port, p->tx_cb_ctx);
    }

    return HAL_CAN_OK;
}

void hal_can_register_rx_callback(hal_can_port_t port, hal_can_rx_cb_t cb,
                                  void *ctx) {
    if (port >= HAL_CAN_PORT_MAX) {
        return;
    }
    can_ports[port].rx_cb = cb;
    can_ports[port].rx_cb_ctx = ctx;
}

void hal_can_register_tx_callback(hal_can_port_t port, hal_can_tx_cb_t cb,
                                  void *ctx) {
    if (port >= HAL_CAN_PORT_MAX) {
        return;
    }

    can_ports[port].tx_cb = cb;
    can_ports[port].tx_cb_ctx = ctx;
}

void hal_can_register_error_callback(hal_can_port_t port, hal_can_error_cb_t cb,
                                     void *ctx) {
    if (port >= HAL_CAN_PORT_MAX) {
        return;
    }

    can_ports[port].err_cb = cb;
    can_ports[port].err_cb_ctx = ctx;
}

void hal_can_isr_handler(hal_can_port_t port) {
    /* mock isr has nothing to do since actions are simulated via functions */
    (void)port;
}

void mock_can_push_rx(hal_can_port_t port, const hal_can_msg_t *msg) {
    if (port >= HAL_CAN_PORT_MAX || msg == NULL) {
        return;
    }

    can_mock_port_t *p = &can_ports[port];
    if (!p->initialized) {
        return;
    }

    /* directly trigger the rx callback to simulate incoming packet interrupt */
    if (p->rx_cb != NULL) {
        p->rx_cb(port, msg, p->rx_cb_ctx);
    }
}

size_t mock_can_get_tx(hal_can_port_t port, hal_can_msg_t *out,
                       size_t max_len) {
    if (port >= HAL_CAN_PORT_MAX || out == NULL || max_len == 0) {
        return 0;
    }

    can_mock_port_t *p = &can_ports[port];
    size_t retrieved = 0;

    while (retrieved < max_len && p->tx_count > 0) {
        out[retrieved] = p->tx_queue[p->tx_tail];
        p->tx_tail = (p->tx_tail + 1) % MOCK_CAN_QUEUE_SIZE;
        p->tx_count--;
        retrieved++;
    }

    return retrieved;
}

void mock_can_reset(void) { memset(can_ports, 0, sizeof(can_ports)); }
