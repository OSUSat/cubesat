#include "hal_i2c_mock.h"
#include <stdio.h>
#include <string.h>

#define MOCK_I2C_BUF_SIZE 256

static uint8_t next_read_buffer[MOCK_I2C_BUF_SIZE];
static uint16_t next_read_len = 0;

static uint8_t last_write_buffer[MOCK_I2C_BUF_SIZE];
static uint16_t last_write_len = 0;
static uint8_t last_write_addr = 0;

void mock_i2c_set_next_read_data(const uint8_t *data, uint16_t len) {
    if (len > MOCK_I2C_BUF_SIZE) {
        len = MOCK_I2C_BUF_SIZE;
    }

    memcpy(next_read_buffer, data, len);

    next_read_len = len;
}

uint16_t mock_i2c_get_last_write(uint8_t *addr_out, uint8_t *buffer,
                                 uint16_t max_len) {
    if (addr_out) {
        *addr_out = last_write_addr;
    }

    uint16_t copy_len = (last_write_len < max_len) ? last_write_len : max_len;

    if (buffer && copy_len > 0) {
        memcpy(buffer, last_write_buffer, copy_len);
    }
    return last_write_len;
}

i2c_status_t hal_i2c_write(i2c_bus_t bus, uint8_t addr, const uint8_t *data,
                           uint16_t len) {
    printf("[HITL] I2C WRITE | Bus: %d | Addr: 0x%02X | Len: %d\n", bus, addr,
           len);

    // capture the write for test verification
    last_write_addr = addr;
    last_write_len = (len < MOCK_I2C_BUF_SIZE) ? len : MOCK_I2C_BUF_SIZE;

    memcpy(last_write_buffer, data, last_write_len);

    return I2C_OK;
}

i2c_status_t hal_i2c_read(i2c_bus_t bus, uint8_t addr, uint8_t *data,
                          uint16_t len) {
    printf("[HITL] I2C READ  | Bus: %d | Addr: 0x%02X | Len: %d\n", bus, addr,
           len);

    if (next_read_len > 0) {
        // return pre-loaded mock data
        uint16_t copy_len = (len < next_read_len) ? len : next_read_len;
        memcpy(data, next_read_buffer, copy_len);

        return I2C_OK;
    }

    // default: return 0s if nothing mocked
    memset(data, 0, len);

    return I2C_OK;
}

i2c_status_t hal_i2c_mem_read(i2c_bus_t bus, uint8_t addr, uint8_t reg,
                              uint8_t *data, uint16_t len) {
    printf("[HITL] I2C MEM READ | Bus: %d | Addr: 0x%02X | Reg: 0x%02X\n", bus,
           addr, reg);

    // for simplicity in simple mocks, redirect to standard read logic
    // or implement specific register logic here.
    return hal_i2c_read(bus, addr, data, len);
}
