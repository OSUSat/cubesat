#ifndef HAL_FLASH_H
#define HAL_FLASH_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum { HAL_FLASH_OK = 0, HAL_FLASH_ERROR } hal_flash_status_t;

void hal_flash_init(void);
hal_flash_status_t hal_flash_write(uint32_t address, const uint8_t *data,
                                   size_t size);
hal_flash_status_t hal_flash_read(uint32_t address, uint8_t *buffer,
                                  size_t size);
hal_flash_status_t hal_flash_erase_sector(uint32_t sector_address);

#endif // HAL_FLASH_H
