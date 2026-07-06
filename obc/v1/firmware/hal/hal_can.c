/**
 * @file hal_can.c
 * @brief CAN bus hardware abstraction implementation wrapping H7 FDCAN.
 */

#include "hal_can.h"
#include "stm32h7xx_hal.h"
#include <string.h>

extern FDCAN_HandleTypeDef hfdcan1;
extern FDCAN_HandleTypeDef hfdcan2;

typedef struct {
    FDCAN_HandleTypeDef *hfdcan;
    hal_can_rx_cb_t rx_cb;
    void *rx_cb_ctx;
    hal_can_tx_cb_t tx_cb;
    void *tx_cb_ctx;
    hal_can_error_cb_t err_cb;
    void *err_cb_ctx;
    bool initialized;
} can_port_state_t;

static can_port_state_t g_can_state[HAL_CAN_PORT_MAX] = {0};

static FDCAN_HandleTypeDef *get_fdcan_handle(hal_can_port_t port) {
    switch (port) {
    case HAL_CAN_PORT_1:
        return &hfdcan1;
    case HAL_CAN_PORT_2:
        return &hfdcan2;
    default:
        return NULL;
    }
}

void hal_can_init(hal_can_port_t port, const hal_can_config_t *config) {
    if (port >= HAL_CAN_PORT_MAX || config == NULL) {
        return;
    }

    can_port_state_t *state = &g_can_state[port];
    state->hfdcan = get_fdcan_handle(port);

    if (state->hfdcan == NULL) {
        return;
    }

    HAL_FDCAN_Start(state->hfdcan);
    HAL_FDCAN_ActivateNotification(state->hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE,
                                   0);

    state->initialized = true;
}

hal_can_status_t hal_can_write(hal_can_port_t port, const hal_can_msg_t *msg) {
    if (port >= HAL_CAN_PORT_MAX || msg == NULL) {
        return HAL_CAN_ERROR;
    }

    can_port_state_t *state = &g_can_state[port];
    if (!state->initialized) {
        return HAL_CAN_ERROR;
    }

    FDCAN_TxHeaderTypeDef tx_header;
    tx_header.Identifier = msg->id;
    tx_header.IdType = (msg->id_type == HAL_CAN_ID_EXT) ? FDCAN_EXTENDED_ID
                                                        : FDCAN_STANDARD_ID;
    tx_header.TxFrameType = (msg->rtr == HAL_CAN_RTR_REMOTE)
                                ? FDCAN_REMOTE_FRAME
                                : FDCAN_DATA_FRAME;

    switch (msg->dlc) {
    case 0:
        tx_header.DataLength = FDCAN_DLC_BYTES_0;
        break;
    case 1:
        tx_header.DataLength = FDCAN_DLC_BYTES_1;
        break;
    case 2:
        tx_header.DataLength = FDCAN_DLC_BYTES_2;
        break;
    case 3:
        tx_header.DataLength = FDCAN_DLC_BYTES_3;
        break;
    case 4:
        tx_header.DataLength = FDCAN_DLC_BYTES_4;
        break;
    case 5:
        tx_header.DataLength = FDCAN_DLC_BYTES_5;
        break;
    case 6:
        tx_header.DataLength = FDCAN_DLC_BYTES_6;
        break;
    case 7:
        tx_header.DataLength = FDCAN_DLC_BYTES_7;
        break;
    case 8:
        tx_header.DataLength = FDCAN_DLC_BYTES_8;
        break;
    default:
        tx_header.DataLength = FDCAN_DLC_BYTES_8;
        break;
    }

    tx_header.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    tx_header.BitRateSwitch = FDCAN_BRS_OFF;
    tx_header.FDFormat = FDCAN_CLASSIC_CAN;
    tx_header.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    tx_header.MessageMarker = 0;

    if (HAL_FDCAN_AddMessageToTxFifoQ(state->hfdcan, &tx_header,
                                      (uint8_t *)msg->data) != HAL_OK) {
        return HAL_CAN_ERROR;
    }

    if (state->tx_cb != NULL) {
        state->tx_cb(port, state->tx_cb_ctx);
    }

    return HAL_CAN_OK;
}

void hal_can_register_rx_callback(hal_can_port_t port, hal_can_rx_cb_t cb,
                                  void *ctx) {
    if (port < HAL_CAN_PORT_MAX) {
        g_can_state[port].rx_cb = cb;
        g_can_state[port].rx_cb_ctx = ctx;
    }
}

void hal_can_register_tx_callback(hal_can_port_t port, hal_can_tx_cb_t cb,
                                  void *ctx) {
    if (port < HAL_CAN_PORT_MAX) {
        g_can_state[port].tx_cb = cb;
        g_can_state[port].tx_cb_ctx = ctx;
    }
}

void hal_can_register_error_callback(hal_can_port_t port, hal_can_error_cb_t cb,
                                     void *ctx) {
    if (port < HAL_CAN_PORT_MAX) {
        g_can_state[port].err_cb = cb;
        g_can_state[port].err_cb_ctx = ctx;
    }
}

void hal_can_isr_handler(hal_can_port_t port) {
    if (port >= HAL_CAN_PORT_MAX) {
        return;
    }
    can_port_state_t *state = &g_can_state[port];
    if (state->initialized) {
        HAL_FDCAN_IRQHandler(state->hfdcan);
    }
}

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan,
                               uint32_t RxFifo0ITs) {
    if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != 0) {
        for (int i = 0; i < HAL_CAN_PORT_MAX; i++) {
            if (g_can_state[i].hfdcan == hfdcan &&
                g_can_state[i].rx_cb != NULL) {
                FDCAN_RxHeaderTypeDef rx_header;
                hal_can_msg_t msg;

                if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &rx_header,
                                           msg.data) == HAL_OK) {
                    msg.id = rx_header.Identifier;
                    msg.id_type = (rx_header.IdType == FDCAN_EXTENDED_ID)
                                      ? HAL_CAN_ID_EXT
                                      : HAL_CAN_ID_STD;
                    msg.rtr = (rx_header.RxFrameType == FDCAN_REMOTE_FRAME)
                                  ? HAL_CAN_RTR_REMOTE
                                  : HAL_CAN_RTR_DATA;

                    switch (rx_header.DataLength) {
                    case FDCAN_DLC_BYTES_0:
                        msg.dlc = 0;
                        break;
                    case FDCAN_DLC_BYTES_1:
                        msg.dlc = 1;
                        break;
                    case FDCAN_DLC_BYTES_2:
                        msg.dlc = 2;
                        break;
                    case FDCAN_DLC_BYTES_3:
                        msg.dlc = 3;
                        break;
                    case FDCAN_DLC_BYTES_4:
                        msg.dlc = 4;
                        break;
                    case FDCAN_DLC_BYTES_5:
                        msg.dlc = 5;
                        break;
                    case FDCAN_DLC_BYTES_6:
                        msg.dlc = 6;
                        break;
                    case FDCAN_DLC_BYTES_7:
                        msg.dlc = 7;
                        break;
                    case FDCAN_DLC_BYTES_8:
                        msg.dlc = 8;
                        break;
                    default:
                        msg.dlc = 8;
                        break;
                    }

                    g_can_state[i].rx_cb(i, &msg, g_can_state[i].rx_cb_ctx);
                }
            }
        }
    }
}
