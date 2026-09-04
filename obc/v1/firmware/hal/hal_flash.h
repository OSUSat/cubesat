/**
 * @file hal_flash.h
 * @brief Flash Memory Hardware Abstraction Layer for OBC.
 *
 * Provides functions for reading, writing, and erasing non-volatile Flash storage.
 */

#ifndef HAL_FLASH_H
#define HAL_FLASH_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @defgroup obc_flash Flash Driver
 * @brief Non-volatile Flash memory storage interface.
 * @{
 */

/**
 * @enum hal_flash_status_t
 * @brief Flash operation status codes.
 */
typedef enum {
    HAL_FLASH_OK = 0, /**< Operation succeeded */
    HAL_FLASH_ERROR   /**< Operation failed */
} hal_flash_status_t;

/**
 * @brief Initialize Flash HAL interface.
 */
void hal_flash_init(void);

/**
 * @brief Write data to Flash memory.
 *
 * @param[in] address Destination address in Flash.
 * @param[in] data Pointer to source data buffer.
 * @param[in] size Number of bytes to write.
 * @return HAL_FLASH_OK on success, HAL_FLASH_ERROR on failure.
 */
hal_flash_status_t hal_flash_write(uint32_t address, const uint8_t *data,
                                    size_t size);

/**
 * @brief Read data from Flash memory.
 *
 * @param[in] address Source address in Flash.
 * @param[out] buffer Pointer to destination output buffer.
 * @param[in] size Number of bytes to read.
 * @return HAL_FLASH_OK on success, HAL_FLASH_ERROR on failure.
 */
hal_flash_status_t hal_flash_read(uint32_t address, uint8_t *buffer,
                                   size_t size);

/**
 * @brief Erase a sector in Flash memory.
 *
 * @param[in] sector_address Base address of sector to erase.
 * @return HAL_FLASH_OK on success, HAL_FLASH_ERROR on failure.
 */
hal_flash_status_t hal_flash_erase_sector(uint32_t sector_address);

/** @} */

#endif // HAL_FLASH_H
