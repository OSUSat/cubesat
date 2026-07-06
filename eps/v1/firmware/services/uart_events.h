/**
 * @file uart_events.h
 * @brief UART Communication Service.
 *
 * Integrates the OSUSat Packet Library with the HAL UART driver.
 */

#ifndef UART_EVENTS_H
#define UART_EVENTS_H

#include "hal_uart.h"
#include "osusat/event_bus.h"
#include "packet.h"
#include <stdbool.h>

/**
 * @defgroup uart_events UART Events Service
 *
 * @{
 */

/**
 * @defgroup uart_events_types Structures
 *
 * @{
 */

#define UART_SERVICE_UID 0xC044

typedef enum {
    /**
     * @brief Published when a valid OSUSatPacket is decoded.
     * Payload: OSUSatPacket
     */
    UART_PACKET_RECEIVED = 0x10,

    /**
     * @brief Published on hardware errors (Overrun, Noise) or CRC failures.
     * Payload: uart_error_t (or generic error code)
     */
    UART_ERROR_DETECTED,

    /**
     * @brief Published when a packet is successfully transmitted.
     * Payload: NULL
     */
    UART_TX_COMPLETE
} uart_event_id_t;

#define UART_EVENT_PACKET_RECEIVED                                             \
    OSUSAT_BUILD_EVENT_ID(UART_SERVICE_UID, UART_PACKET_RECEIVED)
#define UART_EVENT_ERROR_DETECTED                                              \
    OSUSAT_BUILD_EVENT_ID(UART_SERVICE_UID, UART_ERROR_DETECTED)
#define UART_EVENT_TX_COMPLETE                                                 \
    OSUSAT_BUILD_EVENT_ID(UART_SERVICE_UID, UART_TX_COMPLETE)

typedef enum {
    RX_STATE_WAIT_START_BYTE, /**< Waiting until a start byte is received */
    RX_STATE_READ_HEADER,     /**< Currently reading the packet header */
    RX_STATE_READ_PAYLOAD     /**< Currently reading the packet payload */
} rx_state_t;

// max packet size: Start(1) + Header(8) + Payload(255) + CRC(2) = 266
// we round up to 300 for safety.
#define UART_RX_MAX_PACKET_SIZE 300

// number of buffers in the pool.
// 4 buffers gives subscribers 4 full packet intervals to read data.
#define UART_PACKET_POOL_SIZE 4

/**
 * @struct uart_events_t
 * @brief UART Service State Object.
 */
typedef struct {
    bool initialized; /**< Whether the UART events service is initialized */
    uart_port_t port; /**< The UART port the service is acting on */

    uint32_t rx_byte_count;      /**< Telemetry: total received bytes */
    uint32_t rx_packet_count;    /**< Telemetry: total packets decoded */
    uint32_t rx_crc_error_count; /**< Telemetry: total errors counted */

    // we collect bytes here until we have a full frame to pass to
    // osusat_packet_unpack
    uint8_t packet_pool[UART_PACKET_POOL_SIZE][UART_RX_MAX_PACKET_SIZE];
    uint8_t pool_index;    /**< Index of the buffer currently being filled */
    uint16_t decode_index; /**< Write position within the current buffer */

    rx_state_t rx_state;          /**< Current Rx state */
    uint16_t expected_packet_len; /**< The expected packet length */

} uart_events_t;

/** @} */

/**
 * @defgroup uart_events_api Public API
 *
 * @{
 */

/**
 * @brief Initialize the UART service.
 *
 * @param[out] uart Service instance
 * @param[in] port  Physical HAL port to use
 */
void uart_events_init(uart_events_t *uart, uart_port_t port);

/**
 * @brief Queue a packet for transmission.
 *
 * Uses osusat_packet_pack() to serialize before sending.
 *
 * @param[in,out] uart Service instance
 * @param[in] packet Pointer to the packet to send
 */
void uart_events_send_packet(uart_events_t *uart, const OSUSatPacket *packet);

/** @} */
/** @} */

#endif // UART_EVENTS_H
