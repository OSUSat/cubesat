#include "hal_i2c.h"
#include "hal_time.h"
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_i2c.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

// define the handles that the driver expects as externs
I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;
I2C_HandleTypeDef hi2c3;
I2C_HandleTypeDef hi2c4;

HAL_StatusTypeDef HAL_I2C_Init(I2C_HandleTypeDef *hi2c) {
    (void)hi2c;
    return HAL_OK;
}

HAL_StatusTypeDef HAL_I2C_Master_Receive_IT(I2C_HandleTypeDef *hi2c,
                                            uint16_t DevAddress, uint8_t *pData,
                                            uint16_t Size) {
    (void)hi2c;
    (void)DevAddress;
    (void)pData;
    (void)Size;
    return HAL_OK;
}

HAL_StatusTypeDef HAL_I2C_Master_Transmit_IT(I2C_HandleTypeDef *hi2c,
                                             uint16_t DevAddress,
                                             uint8_t *pData, uint16_t Size) {
    (void)hi2c;
    (void)DevAddress;
    (void)pData;
    (void)Size;
    return HAL_OK;
}

HAL_StatusTypeDef HAL_I2C_Mem_Read_IT(I2C_HandleTypeDef *hi2c,
                                      uint16_t DevAddress, uint16_t MemAddress,
                                      uint16_t MemAddSize, uint8_t *pData,
                                      uint16_t Size) {
    (void)hi2c;
    (void)DevAddress;
    (void)MemAddress;
    (void)MemAddSize;
    (void)pData;
    (void)Size;
    return HAL_OK;
}

HAL_StatusTypeDef HAL_I2C_Mem_Write_IT(I2C_HandleTypeDef *hi2c,
                                       uint16_t DevAddress, uint16_t MemAddress,
                                       uint16_t MemAddSize, uint8_t *pData,
                                       uint16_t Size) {
    (void)hi2c;
    (void)DevAddress;
    (void)MemAddress;
    (void)MemAddSize;
    (void)pData;
    (void)Size;
    return HAL_OK;
}

void HAL_I2C_EV_IRQHandler(I2C_HandleTypeDef *hi2c) { (void)hi2c; }

static bool rx_callback_called = false;
static bool error_callback_called = false;
static i2c_error_t last_error = I2C_HAL_ERR_NONE;

static void dummy_rx_cb(i2c_bus_t bus, void *ctx) {
    (void)bus;
    (void)ctx;
    rx_callback_called = true;
}

static void dummy_err_cb(i2c_bus_t bus, i2c_error_t err, void *ctx) {
    (void)bus;
    (void)ctx;
    error_callback_called = true;
    last_error = err;
}

void test_i2c_busy_timeout(void) {
    printf("Running test: %s\n", __func__);

    hal_time_init();
    hal_i2c_init(I2C_BUS_1);

    uint8_t buffer[10] = {0};

    // start an initial read operation
    i2c_error_t err1 = hal_i2c_read(I2C_BUS_1, 0x50, buffer, 10, dummy_rx_cb,
                                    dummy_err_cb, NULL);
    assert(err1 == I2C_HAL_ERR_NONE);

    // immediately trying to read again should fail with busy error
    i2c_error_t err2 = hal_i2c_read(I2C_BUS_1, 0x50, buffer, 10, dummy_rx_cb,
                                    dummy_err_cb, NULL);
    assert(err2 == I2C_HAL_ERR_BUSY);
    assert(!error_callback_called);

    // sleep for 150 milliseconds to trigger software timeout
    usleep(150000);

    // next read attempt should detect timeout, trigger error callback, and
    // start successfully
    i2c_error_t err3 = hal_i2c_read(I2C_BUS_1, 0x50, buffer, 10, dummy_rx_cb,
                                    dummy_err_cb, NULL);
    assert(error_callback_called);
    assert(last_error == I2C_HAL_ERR_TIMEOUT);
    assert(err3 == I2C_HAL_ERR_NONE);

    printf("Test passed.\n");
}

int main(void) {
    test_i2c_busy_timeout();
    return 0;
}
