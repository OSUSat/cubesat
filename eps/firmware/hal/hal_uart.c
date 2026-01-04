/**
 * @file uart.c
 * @brief UART hardware abstraction implementation.
 */

#include "hal_uart.h"
#include "osusat/ring_buffer.h"
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_def.h"
#include "stm32l4xx_hal_uart.h"
#include <stdint.h>
#include <string.h>

/**
 * @brief Internal UART port state
 */
typedef struct {
    UART_HandleTypeDef *huart;            /**< STM32 HAL UART handle */
    osusat_ring_buffer_t rx_ring;         /**< RX ring buffer */
    uint8_t rx_storage[UART_RX_CAPACITY]; /**< RX buffer storage */
    uart_rx_callback_t rx_callback;       /**< User RX callback */
    void *rx_callback_ctx;                /**< User callback context */
    uint8_t rx_byte;                      /**< Single byte for DMA/IT RX */
    bool initialized;                     /**< Init flag */
} uart_port_state_t;

static uart_port_state_t g_uart_state[UART_PORT_MAX] = {0};

extern UART_HandleTypeDef huart1;
// extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
// extern UART_HandleTypeDef huart4;

/**
 * @brief Map uart_port_t to STM32 HAL handle
 */
static UART_HandleTypeDef *get_hal_handle(uart_port_t port) {
    switch (port) {
    case UART_PORT_1:
        return &huart1;
    case UART_PORT_3:
        return &huart3;
    // NOTE: our pinout does not use ports 2 or 4
    default:
        return NULL;
    }
}

/**
 * @brief Start reception for a single byte (used by interrupt-driven RX)
 */
static void start_rx_interrupt(uart_port_t port) {
    uart_port_state_t *state = &g_uart_state[port];

    // start receiving single byte in interrupt mode
    HAL_UART_Receive_IT(state->huart, &state->rx_byte, 1);
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

    start_rx_interrupt(port);

    state->initialized = true;
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

void hal_uart_write(uart_port_t port, const uint8_t *data, uint16_t len) {
    if (port >= UART_PORT_MAX || data == NULL || len == 0) {
        return;
    }

    uart_port_state_t *state = &g_uart_state[port];
    if (!state->initialized) {
        return;
    }

    // NOTE: blocking
    HAL_UART_Transmit(state->huart, (uint8_t *)data, len, HAL_MAX_DELAY);
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

    // let STM32 HAL handle the interrupt
    HAL_UART_IRQHandler(state->huart);
}

/**
 * @brief HAL callback for RX complete (called from interrupt context)
 *
 * This is called by STM32 HAL when a byte is received.
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    uart_port_t port;
    for (port = 0; port < UART_PORT_MAX; port++) {
        if (g_uart_state[port].huart == huart) {
            break;
        }
    }

    if (port >= UART_PORT_MAX) {
        return;
    }

    uart_port_state_t *state = &g_uart_state[port];

    osusat_ring_buffer_push(&state->rx_ring, state->rx_byte);

    if (state->rx_callback != NULL) {
        state->rx_callback(port, state->rx_callback_ctx);
    }

    start_rx_interrupt(port);
}

/**
 * @brief HAL callback for TX complete (optional)
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    // could add TX complete callback here if needed
    (void)huart;
}

/**
 * @brief HAL callback for UART error
 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
    uart_port_t port;
    for (port = 0; port < UART_PORT_MAX; port++) {
        if (g_uart_state[port].huart == huart) {
            break;
        }
    }

    if (port >= UART_PORT_MAX) {
        return;
    }

    uart_port_state_t *state = &g_uart_state[port];

    // TODO: alert redundancy manager or service or something

    // clear error flags and restart reception
    __HAL_UART_CLEAR_FLAG(huart, UART_CLEAR_OREF | UART_CLEAR_NEF |
                                     UART_CLEAR_PEF | UART_CLEAR_FEF);

    start_rx_interrupt(port);
}
