/**
 * @file hal_flash.c
 * @brief Flash memory hardware abstraction implementation for STM32L4.
 */

#include "hal_flash.h"
#include <string.h>

#if defined(__arm__)
#include "stm32l4xx_hal.h"

void hal_flash_init(void) {
    // no-op or standard peripheral preparation
}

hal_flash_status_t hal_flash_write(uint32_t address, const uint8_t *data,
                                   size_t size) {
    HAL_FLASH_Unlock();

    size_t i = 0;
    HAL_StatusTypeDef status = HAL_OK;

    while (i < size) {
        uint64_t data64 = 0xFFFFFFFFFFFFFFFFULL;
        size_t chunk = (size - i >= 8) ? 8 : (size - i);
        memcpy(&data64, &data[i], chunk);

        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address + i,
                                   data64);

        if (status != HAL_OK) {
            break;
        }

        i += 8;
    }

    HAL_FLASH_Lock();

    return (status == HAL_OK) ? HAL_FLASH_OK : HAL_FLASH_ERROR;
}

hal_flash_status_t hal_flash_read(uint32_t address, uint8_t *buffer,
                                  size_t size) {
    // direct memory-mapped read for internal Flash
    memcpy(buffer, (const void *)(uintptr_t)address, size);

    return HAL_FLASH_OK;
}

hal_flash_status_t hal_flash_erase_sector(uint32_t sector_address) {
    HAL_FLASH_Unlock();

    // determine page from address (2KB pages on STM32L496xx)
    uint32_t page = (sector_address - FLASH_BASE) / 2048;
    uint32_t bank = (page < 256) ? FLASH_BANK_1 : FLASH_BANK_2;

    FLASH_EraseInitTypeDef erase_init = {.TypeErase = FLASH_TYPEERASE_PAGES,
                                         .Banks = bank,
                                         .Page = page,
                                         .NbPages = 1};

    uint32_t page_error = 0;
    HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&erase_init, &page_error);

    HAL_FLASH_Lock();

    return (status == HAL_OK) ? HAL_FLASH_OK : HAL_FLASH_ERROR;
}

#endif // defined(__arm__)
