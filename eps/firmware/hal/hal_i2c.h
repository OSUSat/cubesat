/**
 * @file i2c.h
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
 * @{
 */

/**
 * @enum i2c_status_t
 * @brief Result codes for I2C operations
 */
typedef enum {
    I2C_OK = 0,  /**< Operation successful */
    I2C_ERROR,   /**< Generic error */
    I2C_TIMEOUT, /**< Bus timeout */
    I2C_NACK     /**< Device not acknowledged */
} i2c_status_t;

/**
 * @enum i2c_bus_t
 * @brief Identifiers for the available I2C busses
 */
typedef enum { I2C_BUS_1 = 1, I2C_BUS_2, I2C_BUS_3, I2C_BUS_4 } i2c_bus_t;

/** @} */ // end i2c_types

/**
 * @defgroup i2c_api Public API
 * @ingroup i2c
 * @brief External interface for interacting with the I2C driver.
 *
 * @{
 */

/**
 * @brief Write data to an I2C device.
 *
 * @param[in] bus     The I2C bus identifier (I2C_BUS_1, etc.)
 * @param[in] addr    The 7-bit device address.
 * @param[in] data    Pointer to the data buffer to transmit.
 * @param[in] len     Number of bytes to write.
 *
 * @return i2c_status_t Status code.
 */
i2c_status_t hal_i2c_write(i2c_bus_t bus, uint8_t addr, const uint8_t *data,
                           uint16_t len);

/**
 * @brief Read data from an I2C device.
 *
 * @param[in] bus     The I2C bus identifier.
 * @param[in] addr    The 7-bit device address.
 * @param[out] data   Pointer to the buffer to store received data.
 * @param[in] len     Number of bytes to read.
 *
 * @return i2c_status_t Status code.
 */
i2c_status_t hal_i2c_read(i2c_bus_t bus, uint8_t addr, uint8_t *data,
                          uint16_t len);

/**
 * @brief Write to a register, then read data (Common pattern for sensors).
 *
 * @param[in] bus     The I2C bus identifier.
 * @param[in] addr    The 7-bit device address.
 * @param[in] reg     The register address to read from.
 * @param[out] data   Pointer to the buffer to store received data.
 * @param[in] len     Number of bytes to read.
 *
 * @return i2c_status_t Status code.
 */
i2c_status_t hal_i2c_mem_read(i2c_bus_t bus, uint8_t addr, uint8_t reg,
                              uint8_t *data, uint16_t len);

/** @} */ // end i2c_api
/** @} */ // end i2c

#endif // HAL_I2C_H
