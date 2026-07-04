#include "hal_can.h"
#include "can.h"
#include "stm32l4xx_hal.h"
#include <string.h>

#if defined(__arm__)

typedef struct {
    CAN_HandleTypeDef *hcan;
    hal_can_rx_cb_t rx_cb;
    void *rx_cb_ctx;
    hal_can_tx_cb_t tx_cb;
    void *tx_cb_ctx;
    hal_can_error_cb_t err_cb;
    void *err_cb_ctx;
    bool initialized;
} can_port_state_t;

static can_port_state_t g_can_state[HAL_CAN_PORT_MAX];

static CAN_HandleTypeDef *get_hal_handle(hal_can_port_t port) {
    switch (port) {
    case HAL_CAN_PORT_1:
        return &hcan1;
    case HAL_CAN_PORT_2:
        return &hcan2;
    default:
        return NULL;
    }
}

void hal_can_init(hal_can_port_t port, const hal_can_config_t *config) {
    if (port >= HAL_CAN_PORT_MAX || config == NULL) {
        return;
    }

    can_port_state_t *state = &g_can_state[port];
    memset(state, 0, sizeof(*state));

    state->hcan = get_hal_handle(port);
    if (state->hcan == NULL) {
        return;
    }

    /* initialize physical peripheral */
    if (port == HAL_CAN_PORT_1) {
        MX_CAN1_Init();
    } else if (port == HAL_CAN_PORT_2) {
        MX_CAN2_Init();
    }

    if (state->hcan->State == HAL_CAN_STATE_ERROR) {
        return;
    }

    /* configure standard accept-all filter */
    CAN_FilterTypeDef filter_init = {0};
    filter_init.FilterIdHigh = 0x0000;
    filter_init.FilterIdLow = 0x0000;
    filter_init.FilterMaskIdHigh = 0x0000;
    filter_init.FilterMaskIdLow = 0x0000;
    filter_init.FilterFIFOAssignment = CAN_RX_FIFO0;
    filter_init.FilterMode = CAN_FILTERMODE_IDMASK;
    filter_init.FilterScale = CAN_FILTERSCALE_32BIT;
    filter_init.FilterActivation = ENABLE;

    if (port == HAL_CAN_PORT_1) {
        filter_init.FilterBank = 0;
        filter_init.SlaveStartFilterBank = 14;
    } else {
        filter_init.FilterBank = 14;
        filter_init.SlaveStartFilterBank = 14;
    }

    if (HAL_CAN_ConfigFilter(state->hcan, &filter_init) != HAL_OK) {
        return;
    }

    /* activate notifications for interrupt driven rx, tx, and errors */
    HAL_CAN_ActivateNotification(state->hcan, CAN_IT_RX_FIFO0_MSG_PENDING |
                                              CAN_IT_TX_MAILBOX_EMPTY |
                                              CAN_IT_ERROR |
                                              CAN_IT_LAST_ERROR_CODE);

    /* start the CAN peripheral */
    if (HAL_CAN_Start(state->hcan) != HAL_OK) {
        return;
    }

    state->initialized = true;
}

hal_can_status_t hal_can_write(hal_can_port_t port, const hal_can_msg_t *msg) {
    if (port >= HAL_CAN_PORT_MAX || msg == NULL) {
        return HAL_CAN_ERROR;
    }

    can_port_state_t *state = &g_can_state[port];
    if (!state->initialized || state->hcan == NULL) {
        return HAL_CAN_ERROR;
    }

    /* check if mailboxes are full */
    if (HAL_CAN_GetTxMailboxesFreeLevel(state->hcan) == 0) {
        return HAL_CAN_BUSY;
    }

    CAN_TxHeaderTypeDef tx_header = {0};

    if (msg->id_type == HAL_CAN_ID_STD) {
        tx_header.StdId = msg->id;
        tx_header.IDE = CAN_ID_STD;
    } else {
        tx_header.ExtId = msg->id;
        tx_header.IDE = CAN_ID_EXT;
    }

    if (msg->rtr == HAL_CAN_RTR_DATA) {
        tx_header.RTR = CAN_RTR_DATA;
    } else {
        tx_header.RTR = CAN_RTR_REMOTE;
    }

    tx_header.DLC = msg->dlc;
    tx_header.TransmitGlobalTime = DISABLE;

    uint32_t mailbox;
    HAL_StatusTypeDef status = HAL_CAN_AddTxMessage(state->hcan, &tx_header, msg->data, &mailbox);

    if (status == HAL_OK) {
        return HAL_CAN_OK;
    } else if (status == HAL_BUSY) {
        return HAL_CAN_BUSY;
    } else {
        return HAL_CAN_ERROR;
    }
}

void hal_can_register_rx_callback(hal_can_port_t port, hal_can_rx_cb_t cb, void *ctx) {
    if (port >= HAL_CAN_PORT_MAX) {
        return;
    }
    g_can_state[port].rx_cb = cb;
    g_can_state[port].rx_cb_ctx = ctx;
}

void hal_can_register_tx_callback(hal_can_port_t port, hal_can_tx_cb_t cb, void *ctx) {
    if (port >= HAL_CAN_PORT_MAX) {
        return;
    }
    g_can_state[port].tx_cb = cb;
    g_can_state[port].tx_cb_ctx = ctx;
}

void hal_can_register_error_callback(hal_can_port_t port, hal_can_error_cb_t cb, void *ctx) {
    if (port >= HAL_CAN_PORT_MAX) {
        return;
    }
    g_can_state[port].err_cb = cb;
    g_can_state[port].err_cb_ctx = ctx;
}

void hal_can_isr_handler(hal_can_port_t port) {
    if (port >= HAL_CAN_PORT_MAX) {
        return;
    }
    can_port_state_t *state = &g_can_state[port];
    if (state->initialized && state->hcan != NULL) {
        HAL_CAN_IRQHandler(state->hcan);
    }
}

/* HAL CAN library weak callbacks override */

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
    hal_can_port_t port = (hcan->Instance == CAN1) ? HAL_CAN_PORT_1 : HAL_CAN_PORT_2;
    can_port_state_t *state = &g_can_state[port];
    CAN_RxHeaderTypeDef rx_header;
    hal_can_msg_t msg = {0};

    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, msg.data) == HAL_OK) {
        if (rx_header.IDE == CAN_ID_STD) {
            msg.id = rx_header.StdId;
            msg.id_type = HAL_CAN_ID_STD;
        } else {
            msg.id = rx_header.ExtId;
            msg.id_type = HAL_CAN_ID_EXT;
        }

        if (rx_header.RTR == CAN_RTR_DATA) {
            msg.rtr = HAL_CAN_RTR_DATA;
        } else {
            msg.rtr = HAL_CAN_RTR_REMOTE;
        }

        msg.dlc = rx_header.DLC;

        if (state->rx_cb != NULL) {
            state->rx_cb(port, &msg, state->rx_cb_ctx);
        }
    }
}

void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan) {
    hal_can_port_t port = (hcan->Instance == CAN1) ? HAL_CAN_PORT_1 : HAL_CAN_PORT_2;
    can_port_state_t *state = &g_can_state[port];
    if (state->tx_cb != NULL) {
        state->tx_cb(port, state->tx_cb_ctx);
    }
}

void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef *hcan) {
    hal_can_port_t port = (hcan->Instance == CAN1) ? HAL_CAN_PORT_1 : HAL_CAN_PORT_2;
    can_port_state_t *state = &g_can_state[port];
    if (state->tx_cb != NULL) {
        state->tx_cb(port, state->tx_cb_ctx);
    }
}

void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef *hcan) {
    hal_can_port_t port = (hcan->Instance == CAN1) ? HAL_CAN_PORT_1 : HAL_CAN_PORT_2;
    can_port_state_t *state = &g_can_state[port];
    if (state->tx_cb != NULL) {
        state->tx_cb(port, state->tx_cb_ctx);
    }
}

void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan) {
    hal_can_port_t port = (hcan->Instance == CAN1) ? HAL_CAN_PORT_1 : HAL_CAN_PORT_2;
    can_port_state_t *state = &g_can_state[port];
    uint32_t error = HAL_CAN_GetError(hcan);
    if (state->err_cb != NULL) {
        state->err_cb(port, error, state->err_cb_ctx);
    }
}

#endif /* defined(__arm__) */
