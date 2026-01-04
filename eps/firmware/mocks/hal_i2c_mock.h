#ifndef HAL_I2C_MOCK_H
#define HAL_I2C_MOCK_H

#include "hal_i2c.h"
#include <stdint.h>

/**
 * @brief Preloads the mock buffer with data for the next Read operation.
 *  When i2c_read() is called next, it will copy data from this buffer.
 *
 * @param data Pointer to data to stage.
 * @param len  Length of data.
 */
void mock_i2c_set_next_read_data(const uint8_t *data, uint16_t len);

/**
 * @brief Gets the last data written to the mock bus via i2c_write().
 *
 * @param[out] addr_out  Pointer to store the address that was written to.
 * @param[out] buffer    Pointer to copy the written data into.
 * @param[in]  max_len   Max bytes to copy.
 *
 * @return uint16_t      Number of bytes actually written in the last op.
 */
uint16_t mock_i2c_get_last_write(uint8_t *addr_out, uint8_t *buffer,
                                 uint16_t max_len);

#endif // HAL_I2C_MOCK_H
