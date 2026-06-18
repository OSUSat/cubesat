/**
 * @file hal_i2c.c
 * @brief I2C hardware abstraction implementation for STM32H7.
 */

#include "hal_i2c.h"
#include "stm32h7xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

extern I2C_HandleTypeDef hi2c2;

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

    HAL_StatusTypeDef status =
        HAL_I2C_Master_Transmit(hi2c, addr << 1, (uint8_t *)data, len, 100);
    if (status == HAL_OK) {
        if (cb != NULL) {
            cb(bus, ctx);
        }

        return I2C_HAL_ERR_NONE;
    } else {
        if (err_cb != NULL) {
            err_cb(bus, I2C_HAL_ERR_UNKNOWN, ctx);
        }

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

    HAL_StatusTypeDef status = HAL_I2C_Mem_Write(
        hi2c, addr << 1, reg, I2C_MEMADD_SIZE_8BIT, data, len, 100);
    if (status == HAL_OK) {
        if (cb != NULL) {
            cb(bus, ctx);
        }

        return I2C_HAL_ERR_NONE;
    } else {
        if (err_cb != NULL) {
            err_cb(bus, I2C_HAL_ERR_UNKNOWN, ctx);
        }

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

    HAL_StatusTypeDef status =
        HAL_I2C_Master_Receive(hi2c, addr << 1, data, len, 100);
    if (status == HAL_OK) {
        if (cb != NULL) {
            cb(bus, ctx);
        }

        return I2C_HAL_ERR_NONE;
    } else {
        if (err_cb != NULL) {
            err_cb(bus, I2C_HAL_ERR_UNKNOWN, ctx);
        }

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

    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(
        hi2c, addr << 1, reg, I2C_MEMADD_SIZE_8BIT, data, len, 100);
    if (status == HAL_OK) {
        if (cb != NULL) {
            cb(bus, ctx);
        }

        return I2C_HAL_ERR_NONE;
    } else {
        if (err_cb != NULL) {
            err_cb(bus, I2C_HAL_ERR_UNKNOWN, ctx);
        }

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
