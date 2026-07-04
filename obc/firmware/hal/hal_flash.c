/**
 * @file hal_flash.c
 * @brief Flash memory hardware abstraction implementation skeleton for STM32H7.
 */

#include "hal_flash.h"
#include <string.h>

#if defined(__arm__)
#include "stm32h7xx_hal.h"
#endif

void hal_flash_init(void) {
    // no-op
}

hal_flash_status_t hal_flash_write(uint32_t address, const uint8_t *data,
                                   size_t size) {
#if defined(__arm__)
    HAL_FLASH_Unlock();
    // H7 programming is typically done in 256-bit (32 bytes) blocks using
    // FLASH_TYPEPROGRAM_FLASHWORD we will stub this or implement minimal write
    // protection.
    HAL_StatusTypeDef status = HAL_OK;
    // for now, return OK as a placeholder
    HAL_FLASH_Lock();
    return (status == HAL_OK) ? HAL_FLASH_OK : HAL_FLASH_ERROR;
#else
    return HAL_FLASH_OK;
#endif
}

hal_flash_status_t hal_flash_read(uint32_t address, uint8_t *buffer,
                                  size_t size) {
    // direct memory-mapped read for internal Flash
    memcpy(buffer, (const void *)(uintptr_t)address, size);
    return HAL_FLASH_OK;
}

hal_flash_status_t hal_flash_erase_sector(uint32_t sector_address) {
#if defined(__arm__)
    HAL_FLASH_Unlock();
    // stubbed erase sector for H7
    HAL_FLASH_Lock();
    return HAL_FLASH_OK;
#else
    return HAL_FLASH_OK;
#endif
}
