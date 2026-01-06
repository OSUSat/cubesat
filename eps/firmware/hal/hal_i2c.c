/**
 * @file hal_i2c.c
 * @brief I2C hardware abstraction implementation
 *
 * This implementation allows reading and writing to I2C devices
 */

#include "hal_i2c.h"
#include "osusat/ring_buffer.h"
#include "stm32l4xx_hal_i2c.h"
#include "stm32l4xx_hal_uart.h"
#include <stdint.h>

/**
 * @struct i2c_bus_state_t
 * @brief Internal state for a single I2C bus
 */
typedef struct {
    I2C_HandleTypeDef *hi2c; /**< STM32 HAL I2C handle */

    osusat_ring_buffer_t rx_ring;        /**< RX ring buffer */
    uint8_t rx_storage[I2C_RX_CAPACITY]; /**< Storage for RX ring buffer */

    i2c_rx_callback_t rx_callback; /**< User RX callback */
    void *rx_callback_ctx;         /**< User callback context */
    uint8_t rx_byte;               /**< Single byte for interrupt RX */

    i2c_error_cb_t error_callback; /**< User error callback hook */
    void *error_callback_ctx;      /**< Error context */

    bool initialized; /**< Init flag */
} i2c_bus_state_t;

static i2c_bus_state_t g_bus_state[I2C_BUS_COUNT];

extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;
extern I2C_HandleTypeDef hi2c3;
extern I2C_HandleTypeDef hi2c4;

/**
 * @brief Map i2c_bus_t to STM32 HAL I2C handle
 */
static I2C_HandleTypeDef *get_hal_handle(i2c_bus_t bus) {
    switch (bus) {
    case I2C_BUS_1:
        return &hi2c1;
    case I2C_BUS_2:
        return &hi2c2;
    case I2C_BUS_3:
        return &hi2c3;
    case I2C_BUS_4:
        return &hi2c4;
    default:
        return NULL;
    }
}

/**
 * @brief Start reception for a single byte (used by interrupt-driven RX)
 */
static void start_rx_interrupt(i2c_bus_t bus) {
    i2c_bus_state_t *state = &g_bus_state[bus];

    // start receiving single byte in interrupt mode
    HAL_UART_Receive_IT(state->hi2c, &state->rx_byte, 1);
}

void hal_i2c_init(i2c_bus_t bus) {
    if (bus >= I2C_BUS_COUNT) {
        return;
    }

    i2c_bus_state_t *state = &g_bus_state[bus];

    state->hi2c = get_hal_handle(bus);
    if (state->hi2c == NULL) {
        return;
    }

    osusat_ring_buffer_init(&state->rx_ring, state->rx_storage, I2C_RX_CAPACITY,
                            true);
}
