/**
 * @file hal_uart.h
 * @brief Universal Asynchronous Receiver-Transmitter (UART) Hardware Abstraction Layer for OBC.
 *
 * Configures serial communications ports, baud rates, transmit/receive buffers, and interrupts.
 */

#ifndef UART_H
#define UART_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @defgroup obc_uart UART Driver
 * @brief UART serial driver interface.
 * @{
 */

#define UART_RX_CAPACITY 128

/**
 * @enum uart_port_t
 * @brief Hardware UART port identifiers.
 */
typedef enum {
    UART_PORT_1 = 0, /**< USART1 */
    UART_PORT_6,     /**< USART6 */
    UART_PORT_7,     /**< UART7 */
    UART_PORT_MAX    /**< Number of available UART ports */
} uart_port_t;

/**
 * @enum uart_error_t
 * @brief UART error hardware codes.
 */
typedef enum {
    UART_HAL_ERR_OVERRUN, /**< Overrun error */
    UART_HAL_ERR_NOISE,   /**< Noise error */
    UART_HAL_ERR_FRAMING, /**< Framing error */
    UART_HAL_ERR_PARITY,  /**< Parity error */
    UART_HAL_ERR_UNKNOWN, /**< Unknown error */
} uart_error_t;

/**
 * @brief Error callback function type.
 */
typedef void (*uart_hal_error_cb_t)(uart_port_t port, uart_error_t err,
                                    void *ctx);

/**
 * @struct uart_config_t
 * @brief Configuration parameters for UART initialization.
 */
typedef struct {
    uint32_t baudrate; /**< UART baud rate in bps */
} uart_config_t;

/**
 * @brief Reception callback function type.
 */
typedef void (*uart_rx_callback_t)(uart_port_t port, void *ctx);

/**
 * @brief Initialize a UART port.
 *
 * @param[in] port Physical UART port index.
 * @param[in] config Configuration pointer containing baudrate.
 */
void hal_uart_init(uart_port_t port, const uart_config_t *config);

/**
 * @brief Register a callback function for UART receive events.
 *
 * @param[in] port Physical UART port index.
 * @param[in] cb Callback handler function.
 * @param[in] ctx User context pointer.
 */
void hal_uart_register_rx_callback(uart_port_t port, uart_rx_callback_t cb,
                                    void *ctx);

/**
 * @brief Register a callback function for UART error events.
 *
 * @param[in] port Physical UART port index.
 * @param[in] cb Callback handler function.
 * @param[in] ctx User context pointer.
 */
void hal_uart_register_error_callback(uart_port_t port, uart_hal_error_cb_t cb,
                                       void *ctx);

/**
 * @brief Transmit data over UART synchronously.
 *
 * @param[in] port Physical UART port index.
 * @param[in] data Pointer to data bytes to send.
 * @param[in] len Length of data in bytes.
 */
void hal_uart_write(uart_port_t port, const uint8_t *data, uint16_t len);

/**
 * @brief Read received data from UART buffer.
 *
 * @param[in] port Physical UART port index.
 * @param[out] out Destination buffer pointer.
 * @param[in] len Maximum bytes to read.
 * @return Number of bytes actually read.
 */
uint16_t hal_uart_read(uart_port_t port, uint8_t *out, uint16_t len);

/**
 * @brief Interrupt service routine handler for UART events.
 *
 * @param[in] port Physical UART port index.
 */
void hal_uart_isr_handler(uart_port_t port);

/** @} */

#endif // UART_H
