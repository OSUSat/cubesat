#ifndef HAL_FRAM_H
#define HAL_FRAM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum { HAL_FRAM_OK = 0, HAL_FRAM_ERROR } hal_fram_status_t;

/**
 * @brief Initialize the FRAM hardware abstraction layer.
 */
void hal_fram_init(void);

/**
 * @brief Write data to the FRAM memory.
 *
 * @param[in] address Start address to write to (0 to 2047)
 * @param[in] data    Pointer to data buffer to write
 * @param[in] size    Number of bytes to write
 * @return HAL_FRAM_OK on success, HAL_FRAM_ERROR otherwise
 */
hal_fram_status_t hal_fram_write(uint32_t address, const uint8_t *data,
                                 size_t size);

/**
 * @brief Read data from the FRAM memory.
 *
 * @param[in] address Start address to read from (0 to 2047)
 * @param[out] buffer Pointer to buffer where read data will be stored
 * @param[in] size    Number of bytes to read
 * @return HAL_FRAM_OK on success, HAL_FRAM_ERROR otherwise
 */
hal_fram_status_t hal_fram_read(uint32_t address, uint8_t *buffer, size_t size);

typedef void (*hal_fram_callback_t)(hal_fram_status_t status, void *ctx);

/**
 * @brief Write data to the FRAM memory non-blockingly (interrupt-driven).
 *
 * @param[in] address Start address to write to (0 to 2047)
 * @param[in] data    Pointer to data buffer to write
 * @param[in] size    Number of bytes to write
 * @param[in] cb      Callback function to invoke on completion/error (can be NULL)
 * @param[in] ctx     User context passed to callback (can be NULL)
 * @return HAL_FRAM_OK if request successfully queued, HAL_FRAM_ERROR otherwise
 */
hal_fram_status_t hal_fram_write_it(uint32_t address, const uint8_t *data,
                                    size_t size, hal_fram_callback_t cb, void *ctx);

/**
 * @brief Read data from the FRAM memory non-blockingly (interrupt-driven).
 *
 * @param[in] address Start address to read from (0 to 2047)
 * @param[out] buffer Pointer to buffer where read data will be stored
 * @param[in] size    Number of bytes to read
 * @param[in] cb      Callback function to invoke on completion/error (can be NULL)
 * @param[in] ctx     User context passed to callback (can be NULL)
 * @return HAL_FRAM_OK if request successfully queued, HAL_FRAM_ERROR otherwise
 */
hal_fram_status_t hal_fram_read_it(uint32_t address, uint8_t *buffer,
                                   size_t size, hal_fram_callback_t cb, void *ctx);

#if !defined(__arm__)
/**
 * @brief Get a pointer to the host mock memory buffer (for testing).
 */
const uint8_t *hal_fram_get_mock_buffer(void);
#endif

#endif // HAL_FRAM_H
