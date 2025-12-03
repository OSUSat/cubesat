/**
 * @file uart_mock.h
 * @brief Mock utilities for the UART HAL.
 *
 * This module provides additional functions used exclusively during testing.
 * These allow injecting fake UART data, inspecting transmitted bytes,
 * simulating ISR events, and resetting the internal mock state.
 */

#ifndef UART_MOCK_H
#define UART_MOCK_H

#include "uart.h"
#include <stddef.h>
#include <stdint.h>

/**
 * @defgroup uart_mock UART Mock
 * @ingroup uart
 * @brief Testing utilities for the UART module.
 * @{
 */

/**
 * @brief Inject test data into the UART RX buffer.
 *
 * Used to simulate bytes arriving from external devices.
 *
 * @param[in] port UART port receiving data
 * @param[in] data Pointer to buffer of injected bytes
 * @param[in] len Number of bytes to inject
 */
void mock_uart_push_rx(uart_port_t port, const uint8_t *data, size_t len);

/**
 * @brief Simulate a single byte arriving via ISR.
 *
 * Mimics the MCU hardware ISR pushing one byte into the HAL buffer.
 * Invokes the registered RX callback if available.
 *
 * @param[in] port UART port receiving data
 * @param[in] byte The byte to inject
 * @return true if the byte was successfully stored in the RX buffer
 * @return false if the RX buffer was full
 */
bool mock_uart_receive_byte_from_isr(uart_port_t port, uint8_t byte);

/**
 * @brief Retrieve data written to a UART TX buffer.
 *
 * @param[in] port UART port whose TX buffer is queried
 * @param[out] buffer Destination buffer to copy TX data into
 * @param[in] max_len Maximum number of bytes to copy
 *
 * @return Number of bytes copied into @p buffer
 */
size_t mock_uart_get_tx(uart_port_t port, uint8_t *out, size_t max_len);

/**
 * @brief Clear all mock UART buffers.
 *
 * Resets both RX and TX mock state for all UART ports.
 */
void mock_uart_reset(void);

/** @} */ // end uart_mock

#endif // UART_MOCK_H
