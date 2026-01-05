/**
 * @file hal_uart.h
 * @brief UART hardware abstraction public API.
 *
 * This driver provides a simple interface for configuring and using UART
 * peripherals. It allows sending and receiving bytes through a serial
 * interface.
 */

#ifndef UART_H
#define UART_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define UART_RX_CAPACITY 128

/**
 * @defgroup uart UART
 * @brief Handles UART communication.
 *
 * @{
 */

/**
 * @defgroup uart_types Types
 * @ingroup uart
 * @brief Types used by the UART driver.
 *
 * @{
 */

/**
 * @enum uart_port_t
 * @brief Available UART ports.
 */
typedef enum {
    UART_PORT_1 = 0, /**< UART port 1 */
    UART_PORT_2,     /**< UART port 2 */
    UART_PORT_3,     /**< UART port 3 */
    UART_PORT_4,     /**< UART port 4 */
    UART_PORT_MAX    /**< Number of available UART ports */
} uart_port_t;

/**
 * @enum uart_error_t
 * @brief Errors that could occur during UART HAL use
 */

typedef enum {
    UART_HAL_ERR_OVERRUN,
    UART_HAL_ERR_NOISE,
    UART_HAL_ERR_FRAMING,
    UART_HAL_ERR_PARITY,
    UART_HAL_ERR_UNKNOWN,
} uart_error_t;

/**
 * @brief Callback function type upon UART errors.
 *
 * @param[in] port   UART port index
 * @param[in] err    The UART error that occurred
 */
typedef void (*uart_hal_error_cb_t)(uart_port_t port, uart_error_t err,
                                    void *ctx);

/**
 * @struct uart_config_t
 * @brief Configuration parameters for UART peripherals.
 */
typedef struct {
    uint32_t baudrate; /**< UART baud rate */
} uart_config_t;

/**
 * @brief Callback for UART RX events.
 *
 * Executed outside the ISR by the UART service or event bus.
 *
 * @param port   The UART that received data.
 * @param ctx    Caller-specified context pointer.
 */
typedef void (*uart_rx_callback_t)(uart_port_t port, void *ctx);

/** @} */ // end uart_types

/**
 * @defgroup uart_api Public API
 * @ingroup uart
 * @brief External interface for interacting with the UART driver.
 *
 * @{
 */

/**
 * @brief Initialize the UART driver.
 *
 * This must be called before configuring or using any UART port.
 *
 * @param[in] port   UART port index
 * @param[in] config Pointer to the UART configuration
 */
void hal_uart_init(uart_port_t port, const uart_config_t *config);

/**
 * @brief Register a callback that fires whenever new bytes are received.
 *
 * @param[in] port The UART port
 * @param[in] cb   The callback handler
 * @param[in] ctx  Any additional context to pass to the RX callback
 */
void hal_uart_register_rx_callback(uart_port_t port, uart_rx_callback_t cb,
                                   void *ctx);

/**
 * @brief Register a callback function upon a UART error.
 *
 * @param[in] port The UART port
 * @param[in] cb   The callback handler
 * @param[in] ctx  Any additional context to pass to the error callback
 */
void hal_uart_register_error_callback(uart_port_t port, uart_hal_error_cb_t cb,
                                      void *ctx);

/**
 * @brief Send raw bytes (blocking or DMA depending on implementation)
 *
 * @param[in] port The UART port
 * @param[in] data The UART data to write
 * @param[in] len  The length of the payload
 */
void hal_uart_write(uart_port_t port, const uint8_t *data, uint16_t len);

/**
 * @brief Non-blocking read of up to `len` bytes from the RX ring buffer.
 *
 * @param[in]  port The UART port
 * @param[out] out The output buffer for any read UART bytes
 * @param[in]  len The length to read
 */
uint16_t hal_uart_read(uart_port_t port, uint8_t *out, uint16_t len);

/**
 * @brief ISR entry point for real hardware.
 * Should be called from the MCU's UARTx_IRQHandler().
 *
 * @param[in] port The UART port
 */
void hal_uart_isr_handler(uart_port_t port);

/** @} */ // end uart_api

/** @} */ // end uart

#endif // UART_H
