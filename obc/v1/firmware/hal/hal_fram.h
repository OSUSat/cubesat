/**
 * @file hal_fram.h
 * @brief FRAM (Ferroelectric RAM) Hardware Abstraction Layer for OBC.
 *
 * Provides synchronous and interrupt-driven non-blocking read/write functions for FRAM.
 */

#ifndef HAL_FRAM_H
#define HAL_FRAM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @defgroup obc_fram FRAM Driver
 * @brief Ferroelectric RAM interface for persistent OBC logging and telemetry data.
 * @{
 */

/**
 * @enum hal_fram_status_t
 * @brief Status codes for FRAM operations.
 */
typedef enum {
    HAL_FRAM_OK = 0, /**< FRAM operation successful */
    HAL_FRAM_ERROR   /**< FRAM operation error */
} hal_fram_status_t;

/**
 * @brief Initialize the FRAM hardware abstraction layer.
 */
void hal_fram_init(void);

/**
 * @brief Write data to FRAM memory synchronously.
 *
 * @param[in] address Start memory address to write to.
 * @param[in] data    Pointer to data buffer to write.
 * @param[in] size    Number of bytes to write.
 * @return HAL_FRAM_OK on success, HAL_FRAM_ERROR otherwise.
 */
hal_fram_status_t hal_fram_write(uint32_t address, const uint8_t *data,
                                 size_t size);

/**
 * @brief Read data from FRAM memory synchronously.
 *
 * @param[in] address Start memory address to read from.
 * @param[out] buffer Pointer to buffer where read data will be stored.
 * @param[in] size    Number of bytes to read.
 * @return HAL_FRAM_OK on success, HAL_FRAM_ERROR otherwise.
 */
hal_fram_status_t hal_fram_read(uint32_t address, uint8_t *buffer, size_t size);

/**
 * @brief Completion callback function type for asynchronous FRAM operations.
 *
 * @param[in] status Completion status of the transaction.
 * @param[in] ctx User context pointer.
 */
typedef void (*hal_fram_callback_t)(hal_fram_status_t status, void *ctx);

/**
 * @brief Write data to FRAM memory non-blockingly (interrupt-driven).
 *
 * @param[in] address Start memory address to write to.
 * @param[in] data    Pointer to data buffer to write.
 * @param[in] size    Number of bytes to write.
 * @param[in] cb      Callback function to invoke on completion/error (can be NULL).
 * @param[in] ctx     User context passed to callback (can be NULL).
 * @return HAL_FRAM_OK if request successfully queued, HAL_FRAM_ERROR otherwise.
 */
hal_fram_status_t hal_fram_write_it(uint32_t address, const uint8_t *data,
                                    size_t size, hal_fram_callback_t cb, void *ctx);

/**
 * @brief Read data from FRAM memory non-blockingly (interrupt-driven).
 *
 * @param[in] address Start memory address to read from.
 * @param[out] buffer Pointer to buffer where read data will be stored.
 * @param[in] size    Number of bytes to read.
 * @param[in] cb      Callback function to invoke on completion/error (can be NULL).
 * @param[in] ctx     User context passed to callback (can be NULL).
 * @return HAL_FRAM_OK if request successfully queued, HAL_FRAM_ERROR otherwise.
 */
hal_fram_status_t hal_fram_read_it(uint32_t address, uint8_t *buffer,
                                   size_t size, hal_fram_callback_t cb, void *ctx);

#if !defined(__arm__)
/**
 * @brief Get a pointer to the host mock memory buffer (for testing).
 *
 * @return Pointer to internal mock FRAM array.
 */
const uint8_t *hal_fram_get_mock_buffer(void);
#endif

/** @} */

#endif // HAL_FRAM_H
