#ifndef HAL_CAN_H
#define HAL_CAN_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    HAL_CAN_PORT_1 = 0,
    HAL_CAN_PORT_2,
    HAL_CAN_PORT_MAX
} hal_can_port_t;

typedef enum {
    HAL_CAN_OK = 0,
    HAL_CAN_ERROR,
    HAL_CAN_BUSY,
    HAL_CAN_TIMEOUT
} hal_can_status_t;

typedef enum { HAL_CAN_ID_STD = 0, HAL_CAN_ID_EXT } hal_can_id_type_t;

typedef enum { HAL_CAN_RTR_DATA = 0, HAL_CAN_RTR_REMOTE } hal_can_rtr_t;

typedef struct {
    uint32_t id;
    hal_can_id_type_t id_type;
    hal_can_rtr_t rtr;
    uint8_t dlc;
    uint8_t data[8];
} hal_can_msg_t;

typedef struct {
    uint32_t baudrate;
} hal_can_config_t;

typedef void (*hal_can_rx_cb_t)(hal_can_port_t port, const hal_can_msg_t *msg,
                                void *ctx);
typedef void (*hal_can_tx_cb_t)(hal_can_port_t port, void *ctx);
typedef void (*hal_can_error_cb_t)(hal_can_port_t port, uint32_t error,
                                   void *ctx);

void hal_can_init(hal_can_port_t port, const hal_can_config_t *config);
hal_can_status_t hal_can_write(hal_can_port_t port, const hal_can_msg_t *msg);
void hal_can_register_rx_callback(hal_can_port_t port, hal_can_rx_cb_t cb,
                                  void *ctx);
void hal_can_register_tx_callback(hal_can_port_t port, hal_can_tx_cb_t cb,
                                  void *ctx);
void hal_can_register_error_callback(hal_can_port_t port, hal_can_error_cb_t cb,
                                     void *ctx);
void hal_can_isr_handler(hal_can_port_t port);

#endif // HAL_CAN_H
