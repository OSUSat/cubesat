/**
 * @file can_events.c
 * @brief CAN Service Implementation for OBC
 */

#include "can_events.h"
#include "hal_can.h"
#include "logging.h"
#include "osusat/event_bus.h"
#include "osusat/slog.h"
#include "packet.h"
#include "hal_gpio.h"
#include "obc_config.h"
#include <stdint.h>
#include <string.h>

#define MIN_HEADER_BYTES (1 + OSUSAT_HEADER_SIZE)

static void can_process_byte(can_events_t *can, uint8_t byte);
static void on_hal_can_rx(hal_can_port_t port, const hal_can_msg_t *msg, void *ctx);
static void on_hal_can_error(hal_can_port_t port, uint32_t error, void *ctx);
static void on_packet_received(const osusat_event_t *e, void *ctx);

void can_events_init(can_events_t *can, hal_can_port_t port) {
    if (can == NULL) {
        return;
    }

    memset(can, 0, sizeof(can_events_t));

    can->port = port;
    can->rx_state = CAN_RX_STATE_WAIT_START_BYTE;

    hal_can_register_rx_callback(port, on_hal_can_rx, can);
    hal_can_register_error_callback(port, on_hal_can_error, can);

    can->initialized = true;

    // Subscribe to received packets to decode EPS logs
    osusat_event_bus_subscribe(CAN_EVENT_PACKET_RECEIVED, on_packet_received, can);

    LOG_INFO(OBC_COMPONENT_CAN, "CAN events service initialized on port %d", port + 1);
}

void can_events_send_packet(can_events_t *can, const OSUSatPacket *packet) {
    if (!can || !can->initialized || !packet) {
        return;
    }

    uint8_t tx_buf[CAN_RX_MAX_PACKET_SIZE];
    int16_t len = osusat_packet_pack(packet, tx_buf, sizeof(tx_buf));

    if (len > 0) {
        uint16_t offset = 0;
        hal_can_status_t status = HAL_CAN_OK;

        while (offset < len) {
            hal_can_msg_t msg = {0};
            msg.id = CAN_MSG_ID_BASE + packet->destination;
            msg.id_type = HAL_CAN_ID_STD;
            msg.rtr = HAL_CAN_RTR_DATA;

            uint8_t chunk_len = (len - offset > 8) ? 8 : (len - offset);
            msg.dlc = chunk_len;
            memcpy(msg.data, &tx_buf[offset], chunk_len);

            status = hal_can_write(can->port, &msg);

            if (status != HAL_CAN_OK) {
                LOG_ERROR(OBC_COMPONENT_CAN, "CAN write failed with status %d on port %d",
                          status, can->port + 1);

                int err_code = (int)status;
                osusat_event_bus_publish(CAN_EVENT_ERROR_DETECTED, &err_code, sizeof(int));
                return;
            }

            offset += chunk_len;
        }

        if (status == HAL_CAN_OK) {
            osusat_event_bus_publish(CAN_EVENT_TX_COMPLETE, NULL, 0);
        }
    }
}

static void on_hal_can_rx(hal_can_port_t port, const hal_can_msg_t *msg, void *ctx) {
    (void)port;
    can_events_t *can = (can_events_t *)ctx;

    if (can && can->initialized && msg) {
        hal_gpio_toggle(RED_LED_PIN);
        can->rx_byte_count += msg->dlc;
        for (uint8_t i = 0; i < msg->dlc; i++) {
            can_process_byte(can, msg->data[i]);
        }
    }
}

static void on_hal_can_error(hal_can_port_t port, uint32_t error, void *ctx) {
    (void)port;
    (void)ctx;

    int err_code = (int)error;
    osusat_event_bus_publish(CAN_EVENT_ERROR_DETECTED, &err_code, sizeof(int));
}

static void can_process_byte(can_events_t *can, uint8_t byte) {
    uint8_t *current_buf = can->packet_pool[can->pool_index];

    // safety check for buffer overflow
    if (can->decode_index >= CAN_RX_MAX_PACKET_SIZE) {
        can->rx_state = CAN_RX_STATE_WAIT_START_BYTE;
        can->decode_index = 0;
    }

    switch (can->rx_state) {
    case CAN_RX_STATE_WAIT_START_BYTE:
        if (byte == OSUSAT_START_BYTE) {
            current_buf[0] = byte;
            can->decode_index = 1;
            can->rx_state = CAN_RX_STATE_READ_HEADER;
        }
        break;

    case CAN_RX_STATE_READ_HEADER:
        current_buf[can->decode_index++] = byte;

        if (can->decode_index >= MIN_HEADER_BYTES) {
            uint8_t payload_len = current_buf[MIN_HEADER_BYTES - 1];
            can->expected_packet_len = OSUSAT_FRAME_OVERHEAD + payload_len;
            can->rx_state = CAN_RX_STATE_READ_PAYLOAD;
        }
        break;

    case CAN_RX_STATE_READ_PAYLOAD:
        current_buf[can->decode_index++] = byte;

        if (can->decode_index >= can->expected_packet_len) {
            OSUSatPacket rx_packet;
            OSUSatPacketResult res = osusat_packet_unpack(
                &rx_packet, current_buf, can->expected_packet_len);

            if (res == OSUSAT_PACKET_OK) {
                // Publish packet received event
                osusat_event_bus_publish(CAN_EVENT_PACKET_RECEIVED, &rx_packet, sizeof(OSUSatPacket));

                can->rx_packet_count++;
                can->pool_index = (can->pool_index + 1) % CAN_PACKET_POOL_SIZE;

                LOG_INFO(OBC_COMPONENT_CAN,
                         "Successfully decoded a CAN packet of length %d",
                         can->expected_packet_len);
            } else {
                can->rx_crc_error_count++;
                osusat_event_bus_publish(CAN_EVENT_ERROR_DETECTED, &res, sizeof(int));

                LOG_ERROR(OBC_COMPONENT_CAN,
                          "Failed to decode a CAN packet of expected length %d",
                          can->expected_packet_len);
            }

            can->rx_state = CAN_RX_STATE_WAIT_START_BYTE;
            can->decode_index = 0;
        }
        break;
    }
}

/**
 * @brief Handle received packet and parse EPS logs if applicable
 */
static void on_packet_received(const osusat_event_t *e, void *ctx) {
    (void)ctx;
    const OSUSatPacket *packet = (const OSUSatPacket *)e->payload;

    if (packet && packet->message_type == OSUSatMessageType_LOG) {
        uint16_t offset = 0;
        // Parse logs inside the packet payload
        while (offset + sizeof(osusat_slog_entry_t) <= packet->payload_len) {
            osusat_slog_entry_t entry;
            memcpy(&entry, packet->payload + offset, sizeof(osusat_slog_entry_t));

            // Bounds check for the message string
            if (offset + sizeof(osusat_slog_entry_t) + entry.message_len + 1 > packet->payload_len) {
                LOG_ERROR(OBC_COMPONENT_CAN, "Corrupted EPS log payload received");
                break;
            }

            const char *message = (const char *)(packet->payload + offset + sizeof(osusat_slog_entry_t));

            // Write to mock FRAM and print as EPS log
            logging_write_external_log(&entry, message);

            offset += sizeof(osusat_slog_entry_t) + entry.message_len + 1;
        }
    }
}
