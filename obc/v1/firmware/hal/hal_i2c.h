#ifndef HAL_I2C_H
#define HAL_I2C_H

#include <stdbool.h>
#include <stdint.h>

#define I2C_RX_CAPACITY 128

typedef enum {
    I2C_BUS_1 = 1,
    I2C_BUS_2,
    I2C_BUS_3,
    I2C_BUS_4,
    I2C_BUS_COUNT
} i2c_bus_t;

typedef enum {
    I2C_OK = 0,
    I2C_ERROR,
    I2C_TIMEOUT,
    I2C_BUSY,
    I2C_NACK
} i2c_status_t;

typedef enum {
    I2C_HAL_ERR_BUS,
    I2C_HAL_ERR_ARBITRATION,
    I2C_HAL_ERR_NACK,
    I2C_HAL_ERR_OVERRUN,
    I2C_HAL_ERR_TIMEOUT,
    I2C_HAL_ERR_UNKNOWN,
    I2C_HAL_ERR_BUSY,
    I2C_HAL_ERR_TOO_LARGE,
    I2C_HAL_ERR_NONE,
} i2c_error_t;

typedef void (*i2c_rx_callback_t)(i2c_bus_t bus, void *ctx);
typedef void (*i2c_tx_callback_t)(i2c_bus_t bus, void *ctx);
typedef void (*i2c_error_cb_t)(i2c_bus_t bus, i2c_error_t err, void *ctx);

void hal_i2c_init(i2c_bus_t bus);
i2c_error_t hal_i2c_write(i2c_bus_t bus, uint8_t addr, const uint8_t *data,
                          uint16_t len, i2c_tx_callback_t cb,
                          i2c_error_cb_t err_cb, void *ctx);
i2c_error_t hal_i2c_mem_write(i2c_bus_t bus, uint8_t addr, uint8_t reg,
                              uint8_t *data, uint16_t len, i2c_tx_callback_t cb,
                              i2c_error_cb_t err_cb, void *ctx);
i2c_error_t hal_i2c_read(i2c_bus_t bus, uint8_t addr, uint8_t *data,
                         uint16_t len, i2c_rx_callback_t cb,
                         i2c_error_cb_t err_cb, void *ctx);
i2c_error_t hal_i2c_mem_read(i2c_bus_t bus, uint8_t addr, uint8_t reg,
                             uint8_t *data, uint16_t len, i2c_rx_callback_t cb,
                             i2c_error_cb_t err_cb, void *ctx);
void hal_i2c_isr_handler(i2c_bus_t bus);

#endif // HAL_I2C_H
