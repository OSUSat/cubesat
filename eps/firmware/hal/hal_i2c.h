/**
 * @file hal_i2c.h
 * @brief I2C hardware abstraction public API.
 *
 * This driver provides a high-level interface for I2C communication.
 */

#ifndef HAL_I2C_H
#define HAL_I2C_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @defgroup i2c I2C
 * @brief Handles I2C Communication.
 *
 * @{
 */

/**
 * @defgroup i2c_types Types
 * @ingroup i2c
 * @brief Types used by the I2C driver.
 *
 * @{
 */

#define I2C_RX_CAPACITY 128

/**
 * @enum i2c_bus_t
 * @brief Identifiers for the available I2C busses
 */
typedef enum {
    I2C_BUS_1 = 1,
    I2C_BUS_2,
    I2C_BUS_3,
    I2C_BUS_4,
    I2C_BUS_COUNT
} i2c_bus_t;

/**
 * @enum i2c_status_t
 * @brief Result codes for I2C operations
 */
typedef enum {
    I2C_OK = 0,  /**< Operation successful */
    I2C_ERROR,   /**< Generic error */
    I2C_TIMEOUT, /**< Bus timeout */
    I2C_BUSY,    /**< The line is busy */
    I2C_NACK     /**< Device not acknowledged */
} i2c_status_t;

/**
 * @enum i2c_error_t
 * @brief Errors that could occur during I2C HAL use
 */
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

/**
 * @brief Callback for I2C RX events.
 *
 * @param port   The I2C bus that received data.
 * @param ctx    Caller-specified context pointer.
 */
typedef void (*i2c_rx_callback_t)(i2c_bus_t bus, void *ctx);

/**
 * @brief Callback for I2C TX events.
 *
 * @param port   The I2C bus that received data.
 * @param ctx    Caller-specified context pointer.
 */
typedef void (*i2c_tx_callback_t)(i2c_bus_t bus, void *ctx);

/**
 * @brief Callback function type upon I2C errors.
 *
 * @param[in] port   I2C bus index
 * @param[in] err    The I2C error that occurred
 * @param[in] ctx    The error context
 */
typedef void (*i2c_error_cb_t)(i2c_bus_t bus, i2c_error_t err, void *ctx);

/** @} */ // end i2c_types

/**
 * @defgroup i2c_api Public API
 * @ingroup i2c
 * @brief External interface for interacting with the I2C driver.
 *
 * @{
 */

void hal_i2c_init(i2c_bus_t bus);

/**
 * @brief Write data to an I2C device.
 *
 * @param[in] bus     The I2C bus identifier (I2C_BUS_1, etc.)
 * @param[in] addr    The 7-bit device address.
 * @param[in] data    Pointer to the data buffer to transmit.
 * @param[in] len     Number of bytes to write.
 * @param[in] cb      The callback to fire upon completion
 * @param[in] err_cb  The callback to fire upon an error occurring
 * @param[in] ctx     The context to provide to the callback upon firing
 *
 * @return i2c_status_t Status code.
 */
i2c_error_t hal_i2c_write(i2c_bus_t bus, uint8_t addr, const uint8_t *data,
                          uint16_t len, i2c_tx_callback_t cb,
                          i2c_error_cb_t err_cb, void *ctx);

/**
 * @brief Write to a specific register on the I2C device
 *
 * @param[in] bus     The I2C bus identifier.
 * @param[in] addr    The 7-bit device address.
 * @param[in] reg     The register address to write to.
 * @param[out] data   Pointer to the buffer containing data to be transmitted.
 * @param[in] len     Number of bytes to write.
 * @param[in] cb      The callback to fire upon completion
 * @param[in] err_cb  The callback to fire upon an error occurring
 * @param[in] ctx     The context to provide to the callback upon firing
 *
 * @return i2c_status_t Status code.
 */
i2c_error_t hal_i2c_mem_write(i2c_bus_t bus, uint8_t addr, uint8_t reg,
                              uint8_t *data, uint16_t len, i2c_tx_callback_t cb,
                              i2c_error_cb_t err_cb, void *ctx);

/**
 * @brief Read data from an I2C device.
 *
 * @param[in] bus     The I2C bus identifier.
 * @param[in] addr    The 7-bit device address.
 * @param[out] data   Pointer to the buffer to store received data.
 * @param[in] len     Number of bytes to read.
 * @param[in] cb      The callback to fire upon completion
 * @param[in] err_cb  The callback to fire upon an error occurring
 * @param[in] ctx     The context to provide to the callback upon firing
 *
 * @return i2c_status_t Status code.
 */
i2c_error_t hal_i2c_read(i2c_bus_t bus, uint8_t addr, uint8_t *data,
                         uint16_t len, i2c_rx_callback_t cb,
                         i2c_error_cb_t err_cb, void *ctx);

/**
 * @brief Write to a register, then read data (Common pattern for sensors).
 *
 * @param[in] bus     The I2C bus identifier.
 * @param[in] addr    The 7-bit device address.
 * @param[in] reg     The register address to read from.
 * @param[out] data   Pointer to the buffer to store received data.
 * @param[in] len     Number of bytes to read.
 * @param[in] cb      The callback to fire upon completion
 * @param[in] err_cb  The callback to fire upon an error occurring
 * @param[in] ctx     The context to provide to the callback upon firing
 *
 * @return i2c_status_t Status code.
 */
i2c_error_t hal_i2c_mem_read(i2c_bus_t bus, uint8_t addr, uint8_t reg,
                             uint8_t *data, uint16_t len, i2c_rx_callback_t cb,
                             i2c_error_cb_t err_cb, void *ctx);

/**
 * @brief ISR entry point for I2C interrupts
 * Should be called from MCU's I2C_IRQHandler()
 *
 * @param[in] The I2C bus
 */
void hal_i2c_isr_handler(i2c_bus_t bus);

/** @} */ // end i2c_api
/** @} */ // end i2c

#endif // HAL_I2C_H
