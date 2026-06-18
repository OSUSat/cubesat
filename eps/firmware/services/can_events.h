/**
 * @file can_events.h
 * @brief CANBus Communication Service.
 *
 * Integrates the OSUSat Packet Library with the HAL CAN driver.
 */

#ifndef CAN_EVENTS_H
#define CAN_EVENTS_H

#include "hal_can.h"
#include "osusat/event_bus.h"
#include "packet.h"
#include <stdbool.h>

/**
 * @defgroup can_events CAN Events Service
 *
 * @{
 */

#define CAN_SERVICE_UID 0xC0C4

typedef enum {
    /**
     * @brief Published when a valid OSUSatPacket is decoded.
     * Payload: OSUSatPacket
     */
    CAN_PACKET_RECEIVED = 0x10,

    /**
     * @brief Published on hardware errors or CRC/decoding failures.
     * Payload: int (or generic error code)
     */
    CAN_ERROR_DETECTED,

    /**
     * @brief Published when a packet is successfully transmitted.
     * Payload: NULL
     */
    CAN_TX_COMPLETE
} can_event_id_t;

#define CAN_EVENT_PACKET_RECEIVED                                              \
    OSUSAT_BUILD_EVENT_ID(CAN_SERVICE_UID, CAN_PACKET_RECEIVED)
#define CAN_EVENT_ERROR_DETECTED                                               \
    OSUSAT_BUILD_EVENT_ID(CAN_SERVICE_UID, CAN_ERROR_DETECTED)
#define CAN_EVENT_TX_COMPLETE                                                  \
    OSUSAT_BUILD_EVENT_ID(CAN_SERVICE_UID, CAN_TX_COMPLETE)

typedef enum {
    CAN_RX_STATE_WAIT_START_BYTE, /**< Waiting until a start byte is received */
    CAN_RX_STATE_READ_HEADER,     /**< Currently reading the packet header */
    CAN_RX_STATE_READ_PAYLOAD     /**< Currently reading the packet payload */
} can_rx_state_t;

// max packet size: Start(1) + Header(8) + Payload(255) + CRC(2) = 266
// we round up to 300 for safety.
#define CAN_RX_MAX_PACKET_SIZE 300

// number of buffers in the pool.
#define CAN_PACKET_POOL_SIZE 4

// Default CAN ID base for OSUSat packet transport
#define CAN_MSG_ID_BASE 0x100

/**
 * @struct can_events_t
 * @brief CAN Service State Object.
 */
typedef struct {
    bool initialized;    /**< Whether the CAN events service is initialized */
    hal_can_port_t port; /**< The CAN port the service is acting on */

    uint32_t rx_byte_count;      /**< Telemetry: total received bytes */
    uint32_t rx_packet_count;    /**< Telemetry: total packets decoded */
    uint32_t rx_crc_error_count; /**< Telemetry: total errors counted */

    // we collect bytes here until we have a full frame to pass to
    // osusat_packet_unpack
    uint8_t packet_pool[CAN_PACKET_POOL_SIZE][CAN_RX_MAX_PACKET_SIZE];
    uint8_t pool_index;    /**< Index of the buffer currently being filled */
    uint16_t decode_index; /**< Write position within the current buffer */

    can_rx_state_t rx_state;      /**< Current Rx state */
    uint16_t expected_packet_len; /**< The expected packet length */
} can_events_t;

/** @} */

/**
 * @defgroup can_events_api Public API
 *
 * @{
 */

/**
 * @brief Initialize the CAN events service.
 *
 * @param[out] can  Service instance
 * @param[in] port  Physical HAL CAN port to use
 */
void can_events_init(can_events_t *can, hal_can_port_t port);

/**
 * @brief Queue a packet for transmission over CAN.
 *
 * Uses osusat_packet_pack() to serialize before sending.
 *
 * @param[in,out] can Service instance
 * @param[in] packet Pointer to the packet to send
 */
void can_events_send_packet(can_events_t *can, const OSUSatPacket *packet);

/** @} */
/** @} */

#endif // CAN_EVENTS_H
