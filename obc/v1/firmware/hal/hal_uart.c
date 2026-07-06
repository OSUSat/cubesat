/**
 * @file hal_uart.c
 * @brief UART hardware abstraction implementation for STM32H7.
 */

#include "hal_uart.h"
#include "osusat/event_bus.h"
#include "osusat/ring_buffer.h"
#include "stm32h7xx_hal.h"
#include <stdint.h>
#include <string.h>

#define UART_TX_CAPACITY 512

typedef struct {
    UART_HandleTypeDef *huart;

    osusat_ring_buffer_t rx_ring;
    uint8_t rx_storage[UART_RX_CAPACITY];

    osusat_ring_buffer_t tx_ring;
    uint8_t tx_storage[UART_TX_CAPACITY];

    uint8_t rx_byte; // single byte rx buffer for IT
    uint8_t current_tx_byte;

    uart_rx_callback_t rx_callback;
    void *rx_callback_ctx;

    uart_hal_error_cb_t error_callback;
    void *error_callback_ctx;

    bool initialized;
} uart_port_state_t;

static uart_port_state_t g_uart_state[UART_PORT_MAX] = {0};

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart6;
extern UART_HandleTypeDef huart7;

static void uart_start_tx(uart_port_t port) {
    uart_port_state_t *state = &g_uart_state[port];

    if (state->huart->gState != HAL_UART_STATE_READY) {
        return;
    }

    if (osusat_ring_buffer_pop(&state->tx_ring, &state->current_tx_byte)) {
        HAL_UART_Transmit_IT(state->huart, &state->current_tx_byte, 1);
    }
}

static UART_HandleTypeDef *get_hal_handle(uart_port_t port) {
    switch (port) {
    case UART_PORT_1:
        return &huart1;
    case UART_PORT_6:
        return &huart6;
    case UART_PORT_7:
        return &huart7;
    default:
        return NULL;
    }
}

void hal_uart_init(uart_port_t port, const uart_config_t *config) {
    if (port >= UART_PORT_MAX || config == NULL) {
        return;
    }

    uart_port_state_t *state = &g_uart_state[port];

    state->huart = get_hal_handle(port);
    if (state->huart == NULL) {
        return;
    }

    osusat_ring_buffer_init(&state->rx_ring, state->rx_storage,
                            UART_RX_CAPACITY, true);
    osusat_ring_buffer_init(&state->tx_ring, state->tx_storage,
                            UART_TX_CAPACITY, true);

    state->huart->Init.BaudRate = config->baudrate;
    state->huart->Init.WordLength = UART_WORDLENGTH_8B;
    state->huart->Init.StopBits = UART_STOPBITS_1;
    state->huart->Init.Parity = UART_PARITY_NONE;
    state->huart->Init.Mode = UART_MODE_TX_RX;
    state->huart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
    state->huart->Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(state->huart) != HAL_OK) {
        return;
    }

    state->initialized = true;

    // start receiving
    HAL_UART_Receive_IT(state->huart, &state->rx_byte, 1);
}

void hal_uart_register_rx_callback(uart_port_t port, uart_rx_callback_t cb,
                                   void *ctx) {
    if (port >= UART_PORT_MAX) {
        return;
    }

    uart_port_state_t *state = &g_uart_state[port];

    state->rx_callback = cb;
    state->rx_callback_ctx = ctx;
}

void hal_uart_register_error_callback(uart_port_t port, uart_hal_error_cb_t cb,
                                      void *ctx) {
    if (port >= UART_PORT_MAX) {
        return;
    }

    g_uart_state[port].error_callback = cb;
    g_uart_state[port].error_callback_ctx = ctx;
}

void hal_uart_write(uart_port_t port, const uint8_t *data, uint16_t len) {
    if (port >= UART_PORT_MAX || data == NULL || len == 0) {
        return;
    }

    uart_port_state_t *state = &g_uart_state[port];
    if (!state->initialized) {
        return;
    }

    for (uint16_t i = 0; i < len; i++) {
        if (!osusat_ring_buffer_push(&state->tx_ring, data[i])) {
            break; // buffer full
        }
    }

    uart_start_tx(port);
}

uint16_t hal_uart_read(uart_port_t port, uint8_t *out, uint16_t len) {
    if (port >= UART_PORT_MAX || out == NULL || len == 0) {
        return 0;
    }

    uart_port_state_t *state = &g_uart_state[port];
    if (!state->initialized) {
        return 0;
    }

    uint16_t read_count = 0;

    for (uint16_t i = 0; i < len; i++) {
        if (!osusat_ring_buffer_pop(&state->rx_ring, &out[i])) {
            break;
        }

        read_count++;
    }

    return read_count;
}

void hal_uart_isr_handler(uart_port_t port) {
    if (port >= UART_PORT_MAX) {
        return;
    }

    uart_port_state_t *state = &g_uart_state[port];
    if (!state->initialized) {
        return;
    }

    HAL_UART_IRQHandler(state->huart);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    for (int i = 0; i < UART_PORT_MAX; i++) {
        if (g_uart_state[i].huart == huart) {
            uart_port_state_t *state = &g_uart_state[i];

            osusat_ring_buffer_push(&state->rx_ring, state->rx_byte);

            if (state->rx_callback != NULL) {
                state->rx_callback(i, state->rx_callback_ctx);
            }

            HAL_UART_Receive_IT(state->huart, &state->rx_byte, 1);

            return;
        }
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    for (int i = 0; i < UART_PORT_MAX; i++) {
        if (g_uart_state[i].huart == huart) {
            uart_start_tx(i);

            return;
        }
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
    uart_port_t port = UART_PORT_MAX;
    for (int i = 0; i < UART_PORT_MAX; i++) {
        if (g_uart_state[i].huart == huart) {
            port = i;

            break;
        }
    }

    if (port == UART_PORT_MAX) {
        return;
    }

    uart_port_state_t *state = &g_uart_state[port];

    uart_error_t err = UART_HAL_ERR_UNKNOWN;
    uint32_t hal_err = huart->ErrorCode;

    if (hal_err & HAL_UART_ERROR_ORE) {
        err = UART_HAL_ERR_OVERRUN;
    } else if (hal_err & HAL_UART_ERROR_NE) {
        err = UART_HAL_ERR_NOISE;
    } else if (hal_err & HAL_UART_ERROR_FE) {
        err = UART_HAL_ERR_FRAMING;
    } else if (hal_err & HAL_UART_ERROR_PE) {
        err = UART_HAL_ERR_PARITY;
    }

    if (state->error_callback != NULL) {
        state->error_callback(port, err, state->error_callback_ctx);
    }

    // restart IT receive
    HAL_UART_Receive_IT(huart, &state->rx_byte, 1);
}
