/**
 * @file hal_flash_mock.c
 * @brief Mock implementation of the Flash driver HAL for host testing.
 */

#include "hal_flash.h"
#include <stdio.h>
#include <string.h>

#define MOCK_FLASH_SIZE (1024 * 1024) // 1MB mock Flash
static uint8_t mock_flash_storage[MOCK_FLASH_SIZE];

void hal_flash_init(void) {
    memset(mock_flash_storage, 0xFF, sizeof(mock_flash_storage));
}

hal_flash_status_t hal_flash_write(uint32_t address, const uint8_t *data,
                                   size_t size) {
    if (address + size > MOCK_FLASH_SIZE) {
        return HAL_FLASH_ERROR;
    }

    // Simulate flash write: can only write 0s (AND operation against existing
    // bits)
    for (size_t i = 0; i < size; i++) {
        mock_flash_storage[address + i] &= data[i];
    }
    return HAL_FLASH_OK;
}

hal_flash_status_t hal_flash_read(uint32_t address, uint8_t *buffer,
                                  size_t size) {
    if (address + size > MOCK_FLASH_SIZE) {
        return HAL_FLASH_ERROR;
    }
    memcpy(buffer, &mock_flash_storage[address], size);
    return HAL_FLASH_OK;
}

hal_flash_status_t hal_flash_erase_sector(uint32_t sector_address) {
    // Page size is 2KB
    uint32_t page_addr = (sector_address / 2048) * 2048;
    if (page_addr + 2048 > MOCK_FLASH_SIZE) {
        return HAL_FLASH_ERROR;
    }
    memset(&mock_flash_storage[page_addr], 0xFF, 2048);
    return HAL_FLASH_OK;
}
