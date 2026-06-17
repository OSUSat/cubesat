/**
 * @file hal_flash.h
 * @brief Flash memory hardware abstraction public API.
 */

#ifndef HAL_FLASH_H
#define HAL_FLASH_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @enum hal_flash_status_t
 * @brief Result codes for Flash operations.
 */
typedef enum { HAL_FLASH_OK = 0, HAL_FLASH_ERROR } hal_flash_status_t;

/**
 * @brief Initialize the Flash driver.
 */
void hal_flash_init(void);

/**
 * @brief Write data to Flash.
 *
 * @param address Destination Flash address.
 * @param data    Pointer to the data buffer to write.
 * @param size    Number of bytes to write.
 * @return hal_flash_status_t Status code.
 */
hal_flash_status_t hal_flash_write(uint32_t address, const uint8_t *data,
                                   size_t size);

/**
 * @brief Read data from Flash.
 *
 * @param address Source Flash address.
 * @param buffer  Pointer to the destination buffer.
 * @param size    Number of bytes to read.
 * @return hal_flash_status_t Status code.
 */
hal_flash_status_t hal_flash_read(uint32_t address, uint8_t *buffer,
                                  size_t size);

/**
 * @brief Erase a Flash page/sector.
 *
 * @param sector_address Sector/page start address.
 * @return hal_flash_status_t Status code.
 */
hal_flash_status_t hal_flash_erase_sector(uint32_t sector_address);

#endif // HAL_FLASH_H
