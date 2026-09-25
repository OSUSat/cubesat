/**
 * @file hal_can.h
 * @brief Controller Area Network (CAN) Hardware Abstraction Layer for OBC.
 *
 * Configures CAN controllers, message frames, and callback handlers.
 */

#ifndef HAL_CAN_H
#define HAL_CAN_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @defgroup obc_can CAN Driver
 * @brief CANBus driver for OSUSat OBC.
 * @{
 */

/**
 * @enum hal_can_port_t
 * @brief CAN hardware port identifiers.
 */
typedef enum {
    HAL_CAN_PORT_1 = 0, /**< FDCAN1 Port */
    HAL_CAN_PORT_2,     /**< FDCAN2 Port */
    HAL_CAN_PORT_MAX    /**< Maximum CAN port count */
} hal_can_port_t;

/**
 * @enum hal_can_status_t
 * @brief Return status codes for CAN operations.
 */
typedef enum {
    HAL_CAN_OK = 0,   /**< Operation completed successfully */
    HAL_CAN_ERROR,    /**< Hardware or driver error */
    HAL_CAN_BUSY,     /**< Peripheral busy */
    HAL_CAN_TIMEOUT   /**< Operation timed out */
} hal_can_status_t;

/**
 * @enum hal_can_id_type_t
 * @brief CAN identifier format.
 */
typedef enum {
    HAL_CAN_ID_STD = 0, /**< Standard 11-bit identifier */
    HAL_CAN_ID_EXT      /**< Extended 29-bit identifier */
} hal_can_id_type_t;

/**
 * @enum hal_can_rtr_t
 * @brief CAN remote transmission request flag.
 */
typedef enum {
    HAL_CAN_RTR_DATA = 0, /**< Data frame */
    HAL_CAN_RTR_REMOTE    /**< Remote transmission frame */
} hal_can_rtr_t;

/**
 * @struct hal_can_msg_t
 * @brief CAN message frame structure.
 */
typedef struct {
    uint32_t id;              /**< CAN identifier */
    hal_can_id_type_t id_type;/**< Identifier type (standard or extended) */
    hal_can_rtr_t rtr;        /**< Frame type */
    uint8_t dlc;              /**< Data length code (0..8) */
    uint8_t data[8];          /**< Data payload */
} hal_can_msg_t;

/**
 * @struct hal_can_config_t
 * @brief Configuration parameters for CAN initialization.
 */
typedef struct {
    uint32_t baudrate; /**< Bus baud rate in bps */
} hal_can_config_t;

/**
 * @brief Callback function type for received CAN messages.
 */
typedef void (*hal_can_rx_cb_t)(hal_can_port_t port, const hal_can_msg_t *msg,
                                void *ctx);

/**
 * @brief Callback function type for completed CAN transmissions.
 */
typedef void (*hal_can_tx_cb_t)(hal_can_port_t port, void *ctx);

/**
 * @brief Callback function type for CAN bus errors.
 */
typedef void (*hal_can_error_cb_t)(hal_can_port_t port, uint32_t error,
                                    void *ctx);

/**
 * @brief Initialize a CAN hardware port.
 *
 * @param[in] port Physical CAN port index.
 * @param[in] config Configuration pointer containing baudrate settings.
 */
void hal_can_init(hal_can_port_t port, const hal_can_config_t *config);

/**
 * @brief Transmit a CAN message frame over the specified port.
 *
 * @param[in] port Physical CAN port index.
 * @param[in] msg Pointer to CAN message structure to transmit.
 * @return HAL_CAN_OK on success, or appropriate error code.
 */
hal_can_status_t hal_can_write(hal_can_port_t port, const hal_can_msg_t *msg);

/**
 * @brief Register a callback for received CAN frames.
 *
 * @param[in] port Physical CAN port index.
 * @param[in] cb Callback handler function.
 * @param[in] ctx User context pointer.
 */
void hal_can_register_rx_callback(hal_can_port_t port, hal_can_rx_cb_t cb,
                                   void *ctx);

/**
 * @brief Register a callback for completed CAN transmissions.
 *
 * @param[in] port Physical CAN port index.
 * @param[in] cb Callback handler function.
 * @param[in] ctx User context pointer.
 */
void hal_can_register_tx_callback(hal_can_port_t port, hal_can_tx_cb_t cb,
                                   void *ctx);

/**
 * @brief Register a callback for CAN error conditions.
 *
 * @param[in] port Physical CAN port index.
 * @param[in] cb Callback handler function.
 * @param[in] ctx User context pointer.
 */
void hal_can_register_error_callback(hal_can_port_t port, hal_can_error_cb_t cb,
                                      void *ctx);

/**
 * @brief Interrupt service routine entry point for CAN port interrupts.
 *
 * @param[in] port Physical CAN port index.
 */
void hal_can_isr_handler(hal_can_port_t port);

/** @} */

#endif // HAL_CAN_H
