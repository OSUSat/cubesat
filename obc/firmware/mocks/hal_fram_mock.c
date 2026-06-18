/**
 * @file hal_fram_mock.c
 * @brief Mock implementation of the FRAM driver HAL for host testing.
 */

#include "hal_fram.h"
#include <string.h>

#define MOCK_FRAM_SIZE 2048
static uint8_t mock_fram_storage[MOCK_FRAM_SIZE];

void hal_fram_init(void) {
    // clear mock FRAM
    memset(mock_fram_storage, 0, sizeof(mock_fram_storage));
}

hal_fram_status_t hal_fram_write(uint32_t address, const uint8_t *data, size_t size) {
    if (address + size > MOCK_FRAM_SIZE) {
        return HAL_FRAM_ERROR;
    }
    memcpy(&mock_fram_storage[address], data, size);
    return HAL_FRAM_OK;
}

hal_fram_status_t hal_fram_read(uint32_t address, uint8_t *buffer, size_t size) {
    if (address + size > MOCK_FRAM_SIZE) {
        return HAL_FRAM_ERROR;
    }
    memcpy(buffer, &mock_fram_storage[address], size);
    return HAL_FRAM_OK;
}

const uint8_t *hal_fram_get_mock_buffer(void) {
    return mock_fram_storage;
}

hal_fram_status_t hal_fram_write_it(uint32_t address, const uint8_t *data,
                                    size_t size, hal_fram_callback_t cb, void *ctx) {
    hal_fram_status_t status = hal_fram_write(address, data, size);
    if (cb) {
        cb(status, ctx);
    }
    return status;
}

hal_fram_status_t hal_fram_read_it(uint32_t address, uint8_t *buffer,
                                   size_t size, hal_fram_callback_t cb, void *ctx) {
    hal_fram_status_t status = hal_fram_read(address, buffer, size);
    if (cb) {
        cb(status, ctx);
    }
    return status;
}
