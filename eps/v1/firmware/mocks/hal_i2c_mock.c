#include "hal_i2c_mock.h"
#include "hal_i2c.h"
#include <stdio.h>
#include <string.h>

#define MOCK_I2C_BUF_SIZE 256

static uint8_t next_read_buffer[MOCK_I2C_BUF_SIZE];
static uint16_t next_read_len = 0;

static uint8_t last_write_buffer[MOCK_I2C_BUF_SIZE];
static uint16_t last_write_len = 0;
static uint8_t last_write_addr = 0;
static uint8_t last_write_reg = 0;
static bool last_write_was_mem = false;

void mock_i2c_set_next_read_data(const uint8_t *data, uint16_t len) {
    if (len > MOCK_I2C_BUF_SIZE) {
        len = MOCK_I2C_BUF_SIZE;
    }

    memcpy(next_read_buffer, data, len);
    next_read_len = len;
}

uint16_t mock_i2c_get_last_write(uint8_t *addr_out, uint8_t *reg_out,
                                 bool *was_mem, uint8_t *buffer,
                                 uint16_t max_len) {
    if (addr_out) {
        *addr_out = last_write_addr;
    }
    if (reg_out) {
        *reg_out = last_write_reg;
    }
    if (was_mem) {
        *was_mem = last_write_was_mem;
    }

    uint16_t copy_len = (last_write_len < max_len) ? last_write_len : max_len;

    if (buffer && copy_len > 0) {
        memcpy(buffer, last_write_buffer, copy_len);
    }

    return last_write_len;
}

void hal_i2c_init(i2c_bus_t bus) { (void)bus; }

i2c_error_t hal_i2c_write(i2c_bus_t bus, uint8_t addr, const uint8_t *data,
                          uint16_t len, i2c_tx_callback_t cb,
                          i2c_error_cb_t err_cb, void *ctx) {
    (void)bus;
    (void)err_cb;

    printf("[MOCK] I2C WRITE | Addr: 0x%02X | Len: %u\n", addr, len);

    last_write_addr = addr;
    last_write_reg = 0;
    last_write_was_mem = false;
    last_write_len = (len < MOCK_I2C_BUF_SIZE) ? len : MOCK_I2C_BUF_SIZE;

    memcpy(last_write_buffer, data, last_write_len);

    if (cb) {
        cb(bus, ctx);
    }

    return I2C_HAL_ERR_NONE;
}

i2c_error_t hal_i2c_mem_write(i2c_bus_t bus, uint8_t addr, uint8_t reg,
                              uint8_t *data, uint16_t len, i2c_tx_callback_t cb,
                              i2c_error_cb_t err_cb, void *ctx) {
    (void)bus;
    (void)err_cb;

    printf("[MOCK] I2C MEM WRITE | Addr: 0x%02X | Reg: 0x%02X | Len: %u\n",
           addr, reg, len);

    last_write_addr = addr;
    last_write_reg = reg;
    last_write_was_mem = true;
    last_write_len = (len < MOCK_I2C_BUF_SIZE) ? len : MOCK_I2C_BUF_SIZE;

    memcpy(last_write_buffer, data, last_write_len);

    if (cb) {
        cb(bus, ctx);
    }

    return I2C_HAL_ERR_NONE;
}

i2c_error_t hal_i2c_read(i2c_bus_t bus, uint8_t addr, uint8_t *data,
                         uint16_t len, i2c_rx_callback_t cb,
                         i2c_error_cb_t err_cb, void *ctx) {
    (void)addr;
    (void)err_cb;

    printf("[MOCK] I2C READ | Len: %u\n", len);

    if (next_read_len > 0) {
        uint16_t copy_len = (len < next_read_len) ? len : next_read_len;
        memcpy(data, next_read_buffer, copy_len);
    } else {
        memset(data, 0, len);
    }

    if (cb) {
        cb(bus, ctx);
    }

    return I2C_HAL_ERR_NONE;
}

i2c_error_t hal_i2c_mem_read(i2c_bus_t bus, uint8_t addr, uint8_t reg,
                             uint8_t *data, uint16_t len, i2c_rx_callback_t cb,
                             i2c_error_cb_t err_cb, void *ctx) {
    (void)addr;
    (void)reg;
    (void)err_cb;

    printf("[MOCK] I2C MEM READ | Reg: 0x%02X | Len: %u\n", reg, len);

    if (next_read_len > 0) {
        uint16_t copy_len = (len < next_read_len) ? len : next_read_len;
        memcpy(data, next_read_buffer, copy_len);
    } else {
        memset(data, 0, len);
    }

    if (cb) {
        cb(bus, ctx);
    }

    return I2C_HAL_ERR_NONE;
}

void hal_i2c_isr_handler(i2c_bus_t bus) { (void)bus; }
