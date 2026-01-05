/**
 * @file uart.c
 * @brief UART hardware abstraction implementation.
 */

#include "hal_uart.h"
#include "osusat/event_bus.h"
#include "osusat/ring_buffer.h"
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_def.h"
#include "stm32l4xx_hal_uart.h"
#include <stdint.h>
#include <string.h>

// size of the raw DMA buffer
// ideally 2x the max expected packet size to prevent overwrites
#define DMA_BUFFER_SIZE 256

/**
 * @brief Internal UART port state
 */
typedef struct {
    UART_HandleTypeDef *huart; /**< STM32 HAL UART handle */

    osusat_ring_buffer_t rx_ring;         /**< RX ring buffer */
    uint8_t rx_storage[UART_RX_CAPACITY]; /**< RX buffer storage */

    uint8_t dma_buffer[DMA_BUFFER_SIZE];
    uint32_t last_dma_pos; /**< Tracks where we last read from the DMA buffer */

    uart_rx_callback_t rx_callback; /**< User RX callback */
    void *rx_callback_ctx;          /**< User callback context */

    uart_hal_error_cb_t error_callback; /**< User error callback hook */
    void *error_callback_ctx;           /**< Error context */

    bool initialized; /**< Init flag */
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
 * @brief Process new bytes from DMA buffer into User Ring Buffer
 *
 * This calculates how many bytes the DMA has written since we last checked,
 * copies them to the ring buffer, and fires the user callback.
 */
static void process_dma_input(uart_port_t port) {
    uart_port_state_t *state = &g_uart_state[port];

    // calculate current DMA Write Position
    // CNDTR counts down from buffer size.
    // so, pos = size - remaining
    uint32_t current_pos =
        DMA_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(state->huart->hdmarx);

    if (current_pos == state->last_dma_pos) {
        return;
    }

    // copy data from DMA buffer to Ring Buffer
    // we must handle the case where the DMA wrapped around the end of the
    // buffer
    uint32_t start = state->last_dma_pos;

    if (current_pos > start) {
        // standard case
        for (uint32_t i = start; i < current_pos; i++) {
            osusat_ring_buffer_push(&state->rx_ring, state->dma_buffer[i]);
        }
    } else {
        // wrap-around case, end of buffer...
        for (uint32_t i = start; i < DMA_BUFFER_SIZE; i++) {
            osusat_ring_buffer_push(&state->rx_ring, state->dma_buffer[i]);
        }

        // ...then from beginning to current
        for (uint32_t i = 0; i < current_pos; i++) {
            osusat_ring_buffer_push(&state->rx_ring, state->dma_buffer[i]);
        }
    }

    // 4. update position tracker
    state->last_dma_pos = current_pos;

    if (state->rx_callback != NULL) {
        state->rx_callback(port, state->rx_callback_ctx);
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

    state->last_dma_pos = 0;

    // start circular dma
    HAL_UART_Receive_DMA(state->huart, state->dma_buffer, DMA_BUFFER_SIZE);

    // enable "idle line" interrupt
    __HAL_UART_ENABLE_IT(state->huart, UART_IT_IDLE);

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

    if (__HAL_UART_GET_FLAG(state->huart, UART_FLAG_IDLE)) {
        __HAL_UART_CLEAR_IDLEFLAG(state->huart);

        process_dma_input(port);
    }

    HAL_UART_IRQHandler(state->huart);
}

/**
 * @brief DMA Half Transfer Complete (Called by HAL)
 */
void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart) {
    for (int i = 0; i < UART_PORT_MAX; i++) {
        if (g_uart_state[i].huart == huart) {
            process_dma_input(i);
            return;
        }
    }
}

/**
 * @brief DMA Transfer Complete (Called by HAL)
 * This fires when the buffer wraps around to the beginning.
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    for (int i = 0; i < UART_PORT_MAX; i++) {
        if (g_uart_state[i].huart == huart) {
            process_dma_input(i);
            return;
        }
    }
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

    // notify the error hook
    // we do this before restarting, so the service knows we might drop a
    // packet
    if (state->error_callback != NULL) {
        state->error_callback(port, err, state->error_callback_ctx);
    }

    // restart DMA if it stopped
    // the HAL ISR usually disables DMA on severe errors (like ORE).
    // we check if the state is "READY" (which means "Not Busy" / Stopped).
    if (huart->RxState == HAL_UART_STATE_READY) {

        // clear error flags
        __HAL_UART_CLEAR_FLAG(huart, UART_CLEAR_OREF | UART_CLEAR_NEF |
                                         UART_CLEAR_PEF | UART_CLEAR_FEF);

        state->last_dma_pos = 0;

        // restart the circular DMA
        HAL_UART_Receive_DMA(huart, state->dma_buffer, DMA_BUFFER_SIZE);

        // re-enable the Idle Line Interrupt (often disabled by HAL on error)
        __HAL_UART_ENABLE_IT(huart, UART_IT_IDLE);
    }
}
