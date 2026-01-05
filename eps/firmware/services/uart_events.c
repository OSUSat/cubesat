/**
 * @file uart_events.c
 * @brief UART Service Implementation
 */

#include "uart_events.h"
#include "hal_uart.h"
#include "osusat/event_bus.h"
#include "osusat/slog.h"
#include "packet.h"
#include "redundancy_manager.h"
#include <stdint.h>
#include <string.h>

#define UART_PROCESS_INTERVAL_TICKS 1

#define MIN_HEADER_BYTES (1 + OSUSAT_HEADER_SIZE)

/**
 * @brief System Tick Handler.
 * Called automatically by the Event Bus.
 *
 * @param e   The event (SYSTICK).
 * @param ctx The context pointer (points to uart_events_t).
 */
static void uart_handle_tick(const osusat_event_t *e, void *ctx);

static void uart_process_incoming_stream(uart_events_t *uart);
static void uart_process_byte(uart_events_t *uart, uint8_t byte);

/**
 * @brief HAL received hook.
 * Called automatically by the HAL upon Rx.
 *
 * @param port   The UART port
 * @param ctx    The context pointer
 */
static void on_hal_rx_notify(uart_port_t port, void *ctx);

/**
 * @brief HAL error hook.
 * Called automatically by the HAL upon an error occurring.
 *
 * @param port   The UART port
 * @param err    The error
 * @param ctx    The context pointer
 */
static void on_hal_error_notify(uart_port_t port, uart_error_t err, void *ctx);

void uart_events_init(uart_events_t *uart, uart_port_t port) {
    if (uart == NULL) {
        return;
    }

    memset(uart, 0, sizeof(uart_events_t));

    uart->port = port;
    uart->rx_state = RX_STATE_WAIT_START_BYTE;

    hal_uart_register_rx_callback(port, on_hal_rx_notify, uart);
    hal_uart_register_error_callback(port, on_hal_error_notify, uart);

    osusat_event_bus_subscribe(EVENT_SYSTICK, uart_handle_tick, uart);

    uart->initialized = true;
}

void uart_events_send_packet(uart_events_t *uart, const OSUSatPacket *packet) {
    if (!uart || !uart->initialized || !packet) {
        return;
    }

    uint8_t tx_buf[UART_RX_MAX_PACKET_SIZE];

    int16_t len = osusat_packet_pack(packet, tx_buf, sizeof(tx_buf));

    if (len > 0) {
        hal_uart_write(uart->port, tx_buf, (uint16_t)len);

        osusat_event_bus_publish(UART_EVENT_TX_COMPLETE, NULL, 0);
    }
}

static void uart_handle_tick(const osusat_event_t *e, void *ctx) {
    (void)e;

    uart_events_t *manager = (uart_events_t *)ctx;

    if (manager->initialized) {
        uart_process_incoming_stream(manager);
    }
}

/**
 * @brief Drains the HAL Ring Buffer and feeds the State Machine
 */
static void uart_process_incoming_stream(uart_events_t *uart) {
    uint8_t chunk[32];
    uint16_t count;

    while ((count = hal_uart_read(uart->port, chunk, sizeof(chunk))) > 0) {
        uart->rx_byte_count += count;

        for (uint16_t i = 0; i < count; i++) {
            uart_process_byte(uart, chunk[i]);
        }
    }
}

/**
 * @brief Reassembly State Machine
 * Reconstructs a full packet frame from the byte stream.
 */
static void uart_process_byte(uart_events_t *uart, uint8_t byte) {
    uint8_t *current_buf = uart->packet_pool[uart->pool_index];

    // safety check for buffer overflow
    if (uart->decode_index >= UART_RX_MAX_PACKET_SIZE) {
        uart->rx_state = RX_STATE_WAIT_START_BYTE;
        uart->decode_index = 0;
    }

    switch (uart->rx_state) {
    case RX_STATE_WAIT_START_BYTE:
        if (byte == OSUSAT_START_BYTE) {
            current_buf[0] = byte;
            uart->decode_index = 1;
            uart->rx_state = RX_STATE_READ_HEADER;
        }

        break;

    case RX_STATE_READ_HEADER:
        current_buf[uart->decode_index++] = byte;

        if (uart->decode_index >= MIN_HEADER_BYTES) {
            uint8_t payload_len = current_buf[MIN_HEADER_BYTES - 1];

            uart->expected_packet_len = OSUSAT_FRAME_OVERHEAD + payload_len;

            uart->rx_state = RX_STATE_READ_PAYLOAD;
        }

        break;

    case RX_STATE_READ_PAYLOAD:
        current_buf[uart->decode_index++] = byte;

        if (uart->decode_index >= uart->expected_packet_len) {
            OSUSatPacket rx_packet;

            OSUSatPacketResult res = osusat_packet_unpack(
                &rx_packet, current_buf, uart->expected_packet_len);

            if (res == OSUSAT_PACKET_OK) {
                osusat_event_bus_publish(UART_EVENT_PACKET_RECEIVED, &rx_packet,
                                         sizeof(OSUSatPacket));

                uart->rx_packet_count++;

                uart->pool_index =
                    (uart->pool_index + 1) % UART_PACKET_POOL_SIZE;

                LOG_INFO(COMPONENT_UART_PRIMARY,
                         "Successfully decoded a packet of length %d",
                         uart->expected_packet_len);
            } else {
                uart->rx_crc_error_count++;

                osusat_event_bus_publish(UART_EVENT_ERROR_DETECTED, &res,
                                         sizeof(int));

                LOG_ERROR(COMPONENT_UART_PRIMARY,
                          "Failed to decode a packet of expected length %d",
                          uart->expected_packet_len);
            }

            uart->rx_state = RX_STATE_WAIT_START_BYTE;
            uart->decode_index = 0;
        }
    }
}

static void on_hal_rx_notify(uart_port_t port, void *ctx) {
    (void)port;
    (void)ctx;
}

static void on_hal_error_notify(uart_port_t port, uart_error_t err, void *ctx) {
    osusat_event_bus_publish(UART_EVENT_ERROR_DETECTED, &err,
                             sizeof(uart_error_t));
}
