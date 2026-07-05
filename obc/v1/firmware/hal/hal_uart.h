#ifndef UART_H
#define UART_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define UART_RX_CAPACITY 128

typedef enum {
    UART_PORT_1 = 0, /**< USART1 */
    UART_PORT_6,     /**< USART6 */
    UART_PORT_7,     /**< UART7 */
    UART_PORT_MAX    /**< Number of available UART ports */
} uart_port_t;

typedef enum {
    UART_HAL_ERR_OVERRUN,
    UART_HAL_ERR_NOISE,
    UART_HAL_ERR_FRAMING,
    UART_HAL_ERR_PARITY,
    UART_HAL_ERR_UNKNOWN,
} uart_error_t;

typedef void (*uart_hal_error_cb_t)(uart_port_t port, uart_error_t err,
                                    void *ctx);

typedef struct {
    uint32_t baudrate; /**< UART baud rate */
} uart_config_t;

typedef void (*uart_rx_callback_t)(uart_port_t port, void *ctx);

void hal_uart_init(uart_port_t port, const uart_config_t *config);
void hal_uart_register_rx_callback(uart_port_t port, uart_rx_callback_t cb,
                                   void *ctx);
void hal_uart_register_error_callback(uart_port_t port, uart_hal_error_cb_t cb,
                                      void *ctx);
void hal_uart_write(uart_port_t port, const uint8_t *data, uint16_t len);
uint16_t hal_uart_read(uart_port_t port, uint8_t *out, uint16_t len);
void hal_uart_isr_handler(uart_port_t port);

#endif // UART_H
