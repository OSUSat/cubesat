/**
 * @file hal_i2c.h
 * @brief Inter-Integrated Circuit (I2C) Hardware Abstraction Layer for OBC.
 *
 * Supports multi-bus I2C master read/write and memory register operations.
 */

#ifndef HAL_I2C_H
#define HAL_I2C_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @defgroup obc_i2c I2C Driver
 * @brief I2C master driver interface.
 * @{
 */

#define I2C_RX_CAPACITY 128

/**
 * @enum i2c_bus_t
 * @brief I2C physical bus instances.
 */
typedef enum {
    I2C_BUS_1 = 1,   /**< I2C Bus 1 */
    I2C_BUS_2,       /**< I2C Bus 2 */
    I2C_BUS_3,       /**< I2C Bus 3 */
    I2C_BUS_4,       /**< I2C Bus 4 */
    I2C_BUS_COUNT    /**< Total bus count */
} i2c_bus_t;

/**
 * @enum i2c_status_t
 * @brief Return status codes for I2C operations.
 */
typedef enum {
    I2C_OK = 0,   /**< Success */
    I2C_ERROR,    /**< General error */
    I2C_TIMEOUT,  /**< Timeout error */
    I2C_BUSY,     /**< Bus busy */
    I2C_NACK      /**< NACK received */
} i2c_status_t;

/**
 * @enum i2c_error_t
 * @brief Detailed I2C error types.
 */
typedef enum {
    I2C_HAL_ERR_BUS,        /**< Bus fault */
    I2C_HAL_ERR_ARBITRATION,/**< Arbitration lost */
    I2C_HAL_ERR_NACK,       /**< NACK received */
    I2C_HAL_ERR_OVERRUN,    /**< Overrun/underrun */
    I2C_HAL_ERR_TIMEOUT,    /**< Timeout */
    I2C_HAL_ERR_UNKNOWN,    /**< Unknown hardware fault */
    I2C_HAL_ERR_BUSY,       /**< Bus busy */
    I2C_HAL_ERR_TOO_LARGE,  /**< Buffer size exceeds limit */
    I2C_HAL_ERR_NONE        /**< No error */
} i2c_error_t;

/**
 * @brief Callback function type for I2C reception completion.
 */
typedef void (*i2c_rx_callback_t)(i2c_bus_t bus, void *ctx);

/**
 * @brief Callback function type for I2C transmission completion.
 */
typedef void (*i2c_tx_callback_t)(i2c_bus_t bus, void *ctx);

/**
 * @brief Callback function type for I2C error handling.
 */
typedef void (*i2c_error_cb_t)(i2c_bus_t bus, i2c_error_t err, void *ctx);

/**
 * @brief Initialize an I2C bus.
 *
 * @param[in] bus Bus identifier.
 */
void hal_i2c_init(i2c_bus_t bus);

/**
 * @brief Write raw data bytes to an I2C slave device.
 *
 * @param[in] bus Target I2C bus.
 * @param[in] addr 7-bit slave address.
 * @param[in] data Pointer to data buffer.
 * @param[in] len Length of data in bytes.
 * @param[in] cb Transmission callback.
 * @param[in] err_cb Error callback.
 * @param[in] ctx User context pointer.
 * @return I2C_HAL_ERR_NONE on success, or detailed error code.
 */
i2c_error_t hal_i2c_write(i2c_bus_t bus, uint8_t addr, const uint8_t *data,
                          uint16_t len, i2c_tx_callback_t cb,
                          i2c_error_cb_t err_cb, void *ctx);

/**
 * @brief Write data bytes to a specific register on an I2C slave device.
 *
 * @param[in] bus Target I2C bus.
 * @param[in] addr 7-bit slave address.
 * @param[in] reg Register memory address.
 * @param[in] data Pointer to data buffer.
 * @param[in] len Length of data in bytes.
 * @param[in] cb Transmission callback.
 * @param[in] err_cb Error callback.
 * @param[in] ctx User context pointer.
 * @return I2C_HAL_ERR_NONE on success, or detailed error code.
 */
i2c_error_t hal_i2c_mem_write(i2c_bus_t bus, uint8_t addr, uint8_t reg,
                              uint8_t *data, uint16_t len, i2c_tx_callback_t cb,
                              i2c_error_cb_t err_cb, void *ctx);

/**
 * @brief Read raw data bytes from an I2C slave device.
 *
 * @param[in] bus Target I2C bus.
 * @param[in] addr 7-bit slave address.
 * @param[out] data Output buffer for read data.
 * @param[in] len Length of data to read in bytes.
 * @param[in] cb Reception callback.
 * @param[in] err_cb Error callback.
 * @param[in] ctx User context pointer.
 * @return I2C_HAL_ERR_NONE on success, or detailed error code.
 */
i2c_error_t hal_i2c_read(i2c_bus_t bus, uint8_t addr, uint8_t *data,
                         uint16_t len, i2c_rx_callback_t cb,
                         i2c_error_cb_t err_cb, void *ctx);

/**
 * @brief Read data bytes from a specific register on an I2C slave device.
 *
 * @param[in] bus Target I2C bus.
 * @param[in] addr 7-bit slave address.
 * @param[in] reg Register memory address.
 * @param[out] data Output buffer for read data.
 * @param[in] len Length of data to read in bytes.
 * @param[in] cb Reception callback.
 * @param[in] err_cb Error callback.
 * @param[in] ctx User context pointer.
 * @return I2C_HAL_ERR_NONE on success, or detailed error code.
 */
i2c_error_t hal_i2c_mem_read(i2c_bus_t bus, uint8_t addr, uint8_t reg,
                             uint8_t *data, uint16_t len, i2c_rx_callback_t cb,
                             i2c_error_cb_t err_cb, void *ctx);

/**
 * @brief Interrupt service routine handler for I2C events.
 *
 * @param[in] bus Bus identifier.
 */
void hal_i2c_isr_handler(i2c_bus_t bus);

/** @} */

#endif // HAL_I2C_H
