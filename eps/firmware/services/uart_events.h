/**
 * @file uart_events.h
 * @brief UART Communication Service.
 *
 * This module manages the physical communication link with other subsystems
 * (primarily the OBC). It wraps the low-level UART drivers
 * and provides a packet-level interface for the application layer.
 *
 * It is responsible for:
 * - buffering incoming bytes.
 * - decoding raw streams into valid ::OSUSatPacket structures.
 * - encoding and transmitting outgoing packets.
 */

#ifndef UART_EVENTS_H
#define UART_EVENTS_H

#include "packet.h"
#include <stdbool.h>

/**
 * @defgroup uart_events UART Events Service
 * @brief Manages external communication via OSUSat Messaging.
 *
 * This service abstracts the byte-level UART peripheral into a
 * message-level interface. The command handler polls this service
 * to receive instructions, and the telemetry service uses it to
 * ship data out.
 *
 * @{
 */

/**
 * @defgroup uart_events_types Structures
 * @ingroup uart_events
 *
 * @brief Data containers for UART state.
 *
 * @{
 */

/**
 * @struct uart_events_t
 * @brief UART Service State Object.
 *
 * Contains the runtime state of the UART wrapper, including:
 *  - RX/TX Ring Buffers
 *  - Parsing state machine variables
 *  - Error counters
 *  - Initialization flags
 */
typedef struct {
    // TODO: Add internal state here, e.g.:
    // RingBuffer rx_buffer;
    // RingBuffer tx_buffer;
    // uint32_t rx_errors;

    bool initialized;
} uart_events_t;

/** @} */ // end uart_events_types

/**
 * @defgroup uart_events_api Public API
 * @ingroup uart_events
 *
 * @brief Functions for sending and receiving messaging packets.
 *
 * @{
 */

/**
 * @brief Initialize the UART hardware and internal buffers.
 *
 * Configures the UART peripheral and
 * initializes the Ring Buffers within the provided context.
 *
 * @param[out] uart Pointer to the UART service instance to initialize.
 */
void uart_events_init(uart_events_t *uart);

/**
 * @brief Periodic update task.
 *
 * This function handles the low-level driver tasks:
 *  - Processing interrupt flags.
 *  - Parsing the raw RX byte buffer inside the context to identify valid
 * packets.
 *  - Flushing the TX buffer to the hardware.
 *
 * @param[in,out] uart Pointer to the UART service instance.
 */
void uart_events_update(uart_events_t *uart);

/**
 * @brief Check if a valid packet has been received and decoded.
 *
 * @param[in] uart Pointer to the UART service instance.
 *
 * @retval true  A complete ::OSUSatPacket is waiting in the RX queue.
 * @retval false No packets available.
 */
bool uart_events_is_packet_available(uart_events_t *uart);

/**
 * @brief Retrieve the next available packet from the RX queue.
 *
 * Pops the oldest packet from the internal FIFO.
 *
 * @pre Call ::uart_events_is_packet_available to ensure data exists.
 *
 * @param[in,out] uart Pointer to the UART service instance.
 * @param[out] packet Pointer to the destination structure where the data will
 * be copied.
 */
void uart_events_get_packet(uart_events_t *uart, OSUSatPacket *packet);

/**
 * @brief Queue a packet for transmission.
 *
 * Serializes the given ::OSUSatPacket into a byte stream and adds it
 * to the internal TX buffer.
 *
 * @param[in,out] uart Pointer to the UART service instance.
 * @param[in] packet Pointer to the packet to send.
 */
void uart_events_send_packet(uart_events_t *uart, const OSUSatPacket *packet);

/** @} */ // end uart_events_api

/** @} */ // end uart_events

#endif // UART_EVENTS_H
