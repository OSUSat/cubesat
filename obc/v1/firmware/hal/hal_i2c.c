/**
 * @file hal_i2c.c
 * @brief I2C hardware abstraction implementation for STM32H7.
 */

#include "hal_i2c.h"
#include "stm32h7xx_hal.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

extern I2C_HandleTypeDef hi2c2;

typedef struct {
    i2c_tx_callback_t tx_cb;
    i2c_rx_callback_t rx_cb;
    i2c_error_cb_t err_cb;
    void *ctx;
} i2c_bus_state_t;

static i2c_bus_state_t g_i2c_state[I2C_BUS_COUNT];

static I2C_HandleTypeDef *get_hal_handle(i2c_bus_t bus) {
    switch (bus) {
    case I2C_BUS_2:
        return &hi2c2;
    default:
        return NULL;
    }
}

void hal_i2c_init(i2c_bus_t bus) {
    // already initialized in main.c
}

i2c_error_t hal_i2c_write(i2c_bus_t bus, uint8_t addr, const uint8_t *data,
                          uint16_t len, i2c_tx_callback_t cb,
                          i2c_error_cb_t err_cb, void *ctx) {
    I2C_HandleTypeDef *hi2c = get_hal_handle(bus);
    if (hi2c == NULL) {
        return I2C_HAL_ERR_UNKNOWN;
    }

    if (hi2c->State != HAL_I2C_STATE_READY) {
        return I2C_HAL_ERR_BUSY;
    }

    if (cb == NULL && err_cb == NULL) {
        HAL_StatusTypeDef status =
            HAL_I2C_Master_Transmit(hi2c, addr << 1, (uint8_t *)data, len, 100);
        return (status == HAL_OK) ? I2C_HAL_ERR_NONE : I2C_HAL_ERR_UNKNOWN;
    }

    g_i2c_state[bus].tx_cb = cb;
    g_i2c_state[bus].err_cb = err_cb;
    g_i2c_state[bus].ctx = ctx;

    HAL_StatusTypeDef status =
        HAL_I2C_Master_Transmit_IT(hi2c, addr << 1, (uint8_t *)data, len);
    if (status == HAL_OK) {
        return I2C_HAL_ERR_NONE;
    } else if (status == HAL_BUSY) {
        return I2C_HAL_ERR_BUSY;
    } else {
        return I2C_HAL_ERR_UNKNOWN;
    }
}

i2c_error_t hal_i2c_mem_write(i2c_bus_t bus, uint8_t addr, uint8_t reg,
                              uint8_t *data, uint16_t len, i2c_tx_callback_t cb,
                              i2c_error_cb_t err_cb, void *ctx) {
    I2C_HandleTypeDef *hi2c = get_hal_handle(bus);
    if (hi2c == NULL) {
        return I2C_HAL_ERR_UNKNOWN;
    }

    if (hi2c->State != HAL_I2C_STATE_READY) {
        return I2C_HAL_ERR_BUSY;
    }

    if (cb == NULL && err_cb == NULL) {
        HAL_StatusTypeDef status = HAL_I2C_Mem_Write(
            hi2c, addr << 1, reg, I2C_MEMADD_SIZE_8BIT, data, len, 100);
        return (status == HAL_OK) ? I2C_HAL_ERR_NONE : I2C_HAL_ERR_UNKNOWN;
    }

    g_i2c_state[bus].tx_cb = cb;
    g_i2c_state[bus].err_cb = err_cb;
    g_i2c_state[bus].ctx = ctx;

    HAL_StatusTypeDef status = HAL_I2C_Mem_Write_IT(
        hi2c, addr << 1, reg, I2C_MEMADD_SIZE_8BIT, data, len);
    if (status == HAL_OK) {
        return I2C_HAL_ERR_NONE;
    } else if (status == HAL_BUSY) {
        return I2C_HAL_ERR_BUSY;
    } else {
        return I2C_HAL_ERR_UNKNOWN;
    }
}

i2c_error_t hal_i2c_read(i2c_bus_t bus, uint8_t addr, uint8_t *data,
                          uint16_t len, i2c_rx_callback_t cb,
                          i2c_error_cb_t err_cb, void *ctx) {
    I2C_HandleTypeDef *hi2c = get_hal_handle(bus);
    if (hi2c == NULL) {
        return I2C_HAL_ERR_UNKNOWN;
    }

    if (hi2c->State != HAL_I2C_STATE_READY) {
        return I2C_HAL_ERR_BUSY;
    }

    if (cb == NULL && err_cb == NULL) {
        HAL_StatusTypeDef status =
            HAL_I2C_Master_Receive(hi2c, addr << 1, data, len, 100);
        return (status == HAL_OK) ? I2C_HAL_ERR_NONE : I2C_HAL_ERR_UNKNOWN;
    }

    g_i2c_state[bus].rx_cb = cb;
    g_i2c_state[bus].err_cb = err_cb;
    g_i2c_state[bus].ctx = ctx;

    HAL_StatusTypeDef status =
        HAL_I2C_Master_Receive_IT(hi2c, addr << 1, data, len);
    if (status == HAL_OK) {
        return I2C_HAL_ERR_NONE;
    } else if (status == HAL_BUSY) {
        return I2C_HAL_ERR_BUSY;
    } else {
        return I2C_HAL_ERR_UNKNOWN;
    }
}

i2c_error_t hal_i2c_mem_read(i2c_bus_t bus, uint8_t addr, uint8_t reg,
                              uint8_t *data, uint16_t len, i2c_rx_callback_t cb,
                              i2c_error_cb_t err_cb, void *ctx) {
    I2C_HandleTypeDef *hi2c = get_hal_handle(bus);
    if (hi2c == NULL) {
        return I2C_HAL_ERR_UNKNOWN;
    }

    if (hi2c->State != HAL_I2C_STATE_READY) {
        return I2C_HAL_ERR_BUSY;
    }

    if (cb == NULL && err_cb == NULL) {
        HAL_StatusTypeDef status = HAL_I2C_Mem_Read(
            hi2c, addr << 1, reg, I2C_MEMADD_SIZE_8BIT, data, len, 100);
        return (status == HAL_OK) ? I2C_HAL_ERR_NONE : I2C_HAL_ERR_UNKNOWN;
    }

    g_i2c_state[bus].rx_cb = cb;
    g_i2c_state[bus].err_cb = err_cb;
    g_i2c_state[bus].ctx = ctx;

    HAL_StatusTypeDef status = HAL_I2C_Mem_Read_IT(
        hi2c, addr << 1, reg, I2C_MEMADD_SIZE_8BIT, data, len);
    if (status == HAL_OK) {
        return I2C_HAL_ERR_NONE;
    } else if (status == HAL_BUSY) {
        return I2C_HAL_ERR_BUSY;
    } else {
        return I2C_HAL_ERR_UNKNOWN;
    }
}

void hal_i2c_isr_handler(i2c_bus_t bus) {
    I2C_HandleTypeDef *hi2c = get_hal_handle(bus);

    if (hi2c != NULL) {
        HAL_I2C_EV_IRQHandler(hi2c);
        HAL_I2C_ER_IRQHandler(hi2c);
    }
}

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c) {
    i2c_bus_t bus = I2C_BUS_COUNT;
    if (hi2c == &hi2c2) {
        bus = I2C_BUS_2;
    }
    if (bus < I2C_BUS_COUNT && g_i2c_state[bus].tx_cb != NULL) {
        i2c_tx_callback_t cb = g_i2c_state[bus].tx_cb;
        void *ctx = g_i2c_state[bus].ctx;
        g_i2c_state[bus].tx_cb = NULL;
        g_i2c_state[bus].ctx = NULL;
        cb(bus, ctx);
    }
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c) {
    i2c_bus_t bus = I2C_BUS_COUNT;
    if (hi2c == &hi2c2) {
        bus = I2C_BUS_2;
    }
    if (bus < I2C_BUS_COUNT && g_i2c_state[bus].rx_cb != NULL) {
        i2c_rx_callback_t cb = g_i2c_state[bus].rx_cb;
        void *ctx = g_i2c_state[bus].ctx;
        g_i2c_state[bus].rx_cb = NULL;
        g_i2c_state[bus].ctx = NULL;
        cb(bus, ctx);
    }
}

void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c) {
    i2c_bus_t bus = I2C_BUS_COUNT;
    if (hi2c == &hi2c2) {
        bus = I2C_BUS_2;
    }
    if (bus < I2C_BUS_COUNT && g_i2c_state[bus].tx_cb != NULL) {
        i2c_tx_callback_t cb = g_i2c_state[bus].tx_cb;
        void *ctx = g_i2c_state[bus].ctx;
        g_i2c_state[bus].tx_cb = NULL;
        g_i2c_state[bus].ctx = NULL;
        cb(bus, ctx);
    }
}

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c) {
    i2c_bus_t bus = I2C_BUS_COUNT;
    if (hi2c == &hi2c2) {
        bus = I2C_BUS_2;
    }
    if (bus < I2C_BUS_COUNT && g_i2c_state[bus].rx_cb != NULL) {
        i2c_rx_callback_t cb = g_i2c_state[bus].rx_cb;
        void *ctx = g_i2c_state[bus].ctx;
        g_i2c_state[bus].rx_cb = NULL;
        g_i2c_state[bus].ctx = NULL;
        cb(bus, ctx);
    }
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c) {
    i2c_bus_t bus = I2C_BUS_COUNT;
    if (hi2c == &hi2c2) {
        bus = I2C_BUS_2;
    }
    if (bus < I2C_BUS_COUNT && g_i2c_state[bus].err_cb != NULL) {
        i2c_error_cb_t err_cb = g_i2c_state[bus].err_cb;
        void *ctx = g_i2c_state[bus].ctx;
        g_i2c_state[bus].err_cb = NULL;
        g_i2c_state[bus].ctx = NULL;
        
        uint32_t hal_err = HAL_I2C_GetError(hi2c);
        i2c_error_t err = I2C_HAL_ERR_UNKNOWN;
        if (hal_err & HAL_I2C_ERROR_BERR) err = I2C_HAL_ERR_BUS;
        else if (hal_err & HAL_I2C_ERROR_ARLO) err = I2C_HAL_ERR_ARBITRATION;
        else if (hal_err & HAL_I2C_ERROR_AF) err = I2C_HAL_ERR_NACK;
        else if (hal_err & HAL_I2C_ERROR_OVR) err = I2C_HAL_ERR_OVERRUN;
        else if (hal_err & HAL_I2C_ERROR_TIMEOUT) err = I2C_HAL_ERR_TIMEOUT;
        
        err_cb(bus, err, ctx);
    }
}
