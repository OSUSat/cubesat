/**
 * @file hal_i2c.c
 * @brief I2C hardware abstraction implementation
 *
 * This implementation allows reading and writing to I2C devices
 */

#include "hal_i2c.h"
#include "eps_config.h"
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_i2c.h"
#include <stdint.h>

/**
 * @struct i2c_bus_state_t
 * @brief Internal state for a single I2C bus
 */
typedef struct {
    I2C_HandleTypeDef *hi2c; /**< STM32 HAL I2C handle */

    uint8_t rx_buffer[I2C_RX_CAPACITY]; /**< Buffer for interrupt RX */
    uint8_t *rx_user_buffer;            /**< User destination buffer */
    uint16_t rx_len;                    /**< Number of requested bytes */

    i2c_rx_callback_t rx_callback; /**< Current reception RX callback */
    void *rx_callback_ctx;         /**< Current reception context */

    i2c_tx_callback_t tx_callback; /**< Current transaction TX callback */
    void *tx_callback_ctx;         /**< Current transaction context */

    i2c_error_cb_t error_callback; /**< Curretn user error callback hook */
    void *error_callback_ctx;      /**< Error context */

    bool busy;        /**< Whether the line is busy */
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
 * @brief Start reception (used by interrupt-driven RX)
 */
static i2c_status_t start_rx_interrupt(i2c_bus_t bus, uint8_t addr,
                                       uint16_t len) {
    i2c_bus_state_t *state = &g_bus_state[bus];

    if (state->busy) {
        return I2C_BUSY;
    }

    state->rx_len = len;
    state->busy = true;

    HAL_StatusTypeDef status = HAL_I2C_Master_Receive_IT(state->hi2c, addr << 1,
                                                         state->rx_buffer, len);

    return (status == HAL_OK) ? I2C_OK : I2C_ERROR;
}

/**
 * @brief Start reception (used by interrupt-driven RX)
 */
static i2c_status_t start_rx_mem_interrupt(i2c_bus_t bus, uint8_t addr,
                                           uint8_t reg, uint16_t len) {
    i2c_bus_state_t *state = &g_bus_state[bus];

    if (state->busy) {
        return I2C_BUSY;
    }

    state->rx_len = len;
    state->busy = true;

    HAL_StatusTypeDef status = HAL_I2C_Mem_Read_IT(
        state->hi2c, addr, reg, I2C_MEMADD_SIZE_8BIT, state->rx_buffer, len);

    return (status == HAL_OK) ? I2C_OK : I2C_ERROR;
}

/**
 * @brief Start write transmission
 */
static i2c_status_t start_tx_interrupt(i2c_bus_t bus, uint8_t addr,
                                       const uint8_t *data, uint16_t len) {
    i2c_bus_state_t *state = &g_bus_state[bus];

    if (state->busy) {
        return I2C_BUSY;
    }

    state->busy = true;

    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit_IT(
        state->hi2c, addr << 1, (uint8_t *)data, len);

    return (status == HAL_OK) ? I2C_OK : I2C_ERROR;
}

/**
 * @brief Start memory write
 */
static i2c_status_t start_tx_mem_interrupt(i2c_bus_t bus, uint8_t addr,
                                           uint8_t reg, const uint8_t *data,
                                           uint16_t len) {
    i2c_bus_state_t *state = &g_bus_state[bus];

    if (state->busy) {
        return I2C_BUSY;
    }

    state->busy = true;

    HAL_StatusTypeDef status =
        HAL_I2C_Mem_Write_IT(state->hi2c, addr << 1, reg, I2C_MEMADD_SIZE_8BIT,
                             (uint8_t *)data, len);

    return (status == HAL_OK) ? I2C_OK : I2C_ERROR;
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

    state->hi2c->Instance = I2C1;
    state->hi2c->Init.Timing = I2C_TIMING_BITFIELD;
    state->hi2c->Init.OwnAddress1 = 0;
    state->hi2c->Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    state->hi2c->Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    state->hi2c->Init.OwnAddress2 = 0;
    state->hi2c->Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    state->hi2c->Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    state->hi2c->Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    if (HAL_I2C_Init(state->hi2c) != HAL_OK) {
        return;
    }

    state->initialized = true;
}

i2c_error_t hal_i2c_read(i2c_bus_t bus, uint8_t addr, uint8_t *data,
                         uint16_t len, i2c_rx_callback_t cb,
                         i2c_error_cb_t err_cb, void *ctx) {
    if (bus >= I2C_BUS_COUNT || data == NULL || len == 0) {
        return I2C_HAL_ERR_UNKNOWN;
    }

    i2c_bus_state_t *state = &g_bus_state[bus];

    if (!state->initialized) {
        return I2C_HAL_ERR_UNKNOWN;
    }

    if (state->busy) {
        return I2C_HAL_ERR_BUSY;
    }

    if (len > I2C_RX_CAPACITY) {
        return I2C_HAL_ERR_TOO_LARGE;
    }

    state->rx_callback = cb;
    state->rx_callback_ctx = ctx;
    state->error_callback = err_cb;
    state->error_callback_ctx = ctx;
    state->rx_user_buffer = data;

    for (uint16_t i = 0; i < len; i++) {
        state->rx_buffer[i] = 0;
    }

    i2c_status_t status = start_rx_interrupt(bus, addr, len);
    if (status != I2C_OK) {
        return I2C_HAL_ERR_BUSY;
    }

    return I2C_HAL_ERR_NONE;
}

i2c_error_t hal_i2c_mem_read(i2c_bus_t bus, uint8_t addr, uint8_t reg,
                             uint8_t *data, uint16_t len, i2c_rx_callback_t cb,
                             i2c_error_cb_t err_cb, void *ctx) {
    if (bus >= I2C_BUS_COUNT || data == NULL || len == 0) {
        return I2C_HAL_ERR_UNKNOWN;
    }

    i2c_bus_state_t *state = &g_bus_state[bus];

    if (!state->initialized) {
        return I2C_HAL_ERR_UNKNOWN;
    }

    if (state->busy) {
        return I2C_HAL_ERR_BUSY;
    }

    if (len > I2C_RX_CAPACITY) {
        return I2C_HAL_ERR_TOO_LARGE;
    }

    state->rx_callback = cb;
    state->rx_callback_ctx = ctx;
    state->error_callback = err_cb;
    state->error_callback_ctx = ctx;
    state->rx_user_buffer = data;

    for (uint16_t i = 0; i < len; i++) {
        state->rx_buffer[i] = 0;
    }

    i2c_status_t status = start_rx_mem_interrupt(bus, addr, reg, len);
    if (status != I2C_OK) {
        return I2C_HAL_ERR_UNKNOWN;
    }

    return I2C_HAL_ERR_NONE;
}

i2c_error_t hal_i2c_write(i2c_bus_t bus, uint8_t addr, const uint8_t *data,
                          uint16_t len, i2c_tx_callback_t cb,
                          i2c_error_cb_t err_cb, void *ctx) {
    if (bus >= I2C_BUS_COUNT || data == NULL || len == 0) {
        return I2C_HAL_ERR_UNKNOWN;
    }

    i2c_bus_state_t *state = &g_bus_state[bus];

    if (!state->initialized) {
        return I2C_HAL_ERR_UNKNOWN;
    }

    if (state->busy) {
        return I2C_HAL_ERR_BUSY;
    }

    state->tx_callback = cb;
    state->tx_callback_ctx = ctx;
    state->error_callback = err_cb;
    state->error_callback_ctx = ctx;

    i2c_status_t status = start_tx_interrupt(bus, addr, data, len);
    if (status != I2C_OK) {
        return I2C_HAL_ERR_UNKNOWN;
    }

    return I2C_HAL_ERR_NONE;
}

i2c_error_t hal_i2c_mem_write(i2c_bus_t bus, uint8_t addr, uint8_t reg,
                              uint8_t *data, uint16_t len, i2c_tx_callback_t cb,
                              i2c_error_cb_t err_cb, void *ctx) {
    if (bus >= I2C_BUS_COUNT || data == NULL || len == 0) {
        return I2C_HAL_ERR_UNKNOWN;
    }

    i2c_bus_state_t *state = &g_bus_state[bus];

    if (!state->initialized) {
        return I2C_HAL_ERR_UNKNOWN;
    }

    if (state->busy) {
        return I2C_HAL_ERR_BUSY;
    }

    state->tx_callback = cb;
    state->tx_callback_ctx = ctx;
    state->error_callback = err_cb;
    state->error_callback_ctx = ctx;

    i2c_status_t status = start_tx_mem_interrupt(bus, addr, reg, data, len);
    if (status != I2C_OK) {
        return I2C_HAL_ERR_UNKNOWN;
    }

    return I2C_HAL_ERR_NONE;
}

void hal_i2c_register_error_callback(i2c_bus_t bus, i2c_error_cb_t cb,
                                     void *ctx) {
    if (bus >= I2C_BUS_COUNT) {
        return;
    }

    i2c_bus_state_t *state = &g_bus_state[bus];

    state->error_callback = cb;
    state->error_callback_ctx = ctx;
}

void hal_i2c_isr_handler(i2c_bus_t bus) {
    if (bus >= I2C_BUS_COUNT) {
        return;
    }

    i2c_bus_state_t *state = &g_bus_state[bus];
    if (!state->initialized) {
        return;
    }

    HAL_I2C_EV_IRQHandler(state->hi2c);
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c) {
    i2c_bus_t bus;
    for (bus = 0; bus < I2C_BUS_COUNT; bus++) {
        if (g_bus_state[bus].hi2c == hi2c) {
            break;
        }
    }

    if (bus >= I2C_BUS_COUNT) {
        return;
    }

    i2c_bus_state_t *state = &g_bus_state[bus];

    for (uint16_t i = 0; i < state->rx_len; i++) {
        state->rx_user_buffer[i] = state->rx_buffer[i];
    }

    state->busy = false;

    if (state->rx_callback) {
        state->rx_callback(bus, state->rx_callback_ctx);
    }
}

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c) {
    i2c_bus_t bus;
    for (bus = 0; bus < I2C_BUS_COUNT; bus++) {
        if (g_bus_state[bus].hi2c == hi2c) {
            break;
        }
    }

    if (bus >= I2C_BUS_COUNT) {
        return;
    }

    i2c_bus_state_t *state = &g_bus_state[bus];
    state->busy = false;

    if (state->tx_callback) {
        state->tx_callback(bus, state->tx_callback_ctx);
    }
}

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c) {
    HAL_I2C_MasterRxCpltCallback(hi2c);
}

void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c) {
    HAL_I2C_MasterTxCpltCallback(hi2c);
}

static i2c_error_t hal_error_to_i2c_error(uint32_t hal_err) {
    if (hal_err & HAL_I2C_ERROR_BERR)
        return I2C_HAL_ERR_BUS;
    if (hal_err & HAL_I2C_ERROR_ARLO)
        return I2C_HAL_ERR_ARBITRATION;
    if (hal_err & HAL_I2C_ERROR_AF)
        return I2C_HAL_ERR_NACK;
    if (hal_err & HAL_I2C_ERROR_OVR)
        return I2C_HAL_ERR_OVERRUN;
    if (hal_err & HAL_I2C_ERROR_TIMEOUT)
        return I2C_HAL_ERR_TIMEOUT;
    if (hal_err & HAL_I2C_ERROR_DMA)
        return I2C_HAL_ERR_UNKNOWN;
    return I2C_HAL_ERR_UNKNOWN;
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c) {
    i2c_bus_t bus;
    for (bus = 0; bus < I2C_BUS_COUNT; bus++) {
        if (g_bus_state[bus].hi2c == hi2c) {
            break;
        }
    }

    if (bus >= I2C_BUS_COUNT) {
        return;
    }

    i2c_bus_state_t *state = &g_bus_state[bus];
    state->busy = false;

    i2c_error_t err = hal_error_to_i2c_error(hi2c->ErrorCode);

    if (state->error_callback) {
        state->error_callback(bus, err, state->error_callback_ctx);
    }
}
