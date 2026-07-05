/**
 * @file hal_can.h
 * @brief CAN bus hardware abstraction public API.
 *
 * This driver provides a high-level interface for CAN communication.
 */

#ifndef HAL_CAN_H
#define HAL_CAN_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @defgroup can CAN
 * @brief Handles CAN Communication.
 *
 * @{
 */

/**
 * @defgroup can_types Types
 * @ingroup can
 * @brief Types used by the CAN driver.
 *
 * @{
 */

/**
 * @enum hal_can_port_t
 * @brief Identifiers for the available CAN ports.
 */
typedef enum {
    HAL_CAN_PORT_1 = 0,
    HAL_CAN_PORT_2,
    HAL_CAN_PORT_MAX
} hal_can_port_t;

/**
 * @enum hal_can_status_t
 * @brief Result codes for CAN operations.
 */
typedef enum {
    HAL_CAN_OK = 0,  /**< Operation successful */
    HAL_CAN_ERROR,   /**< Generic error */
    HAL_CAN_BUSY,    /**< All Tx mailboxes are full */
    HAL_CAN_TIMEOUT  /**< Operation timed out */
} hal_can_status_t;

/**
 * @enum hal_can_id_type_t
 * @brief Identifiers for standard or extended ID types.
 */
typedef enum {
    HAL_CAN_ID_STD = 0, /**< Standard 11-bit identifier */
    HAL_CAN_ID_EXT      /**< Extended 29-bit identifier */
} hal_can_id_type_t;

/**
 * @enum hal_can_rtr_t
 * @brief Identifiers for frame types.
 */
typedef enum {
    HAL_CAN_RTR_DATA = 0, /**< Data frame */
    HAL_CAN_RTR_REMOTE    /**< Remote transmission request frame */
} hal_can_rtr_t;

/**
 * @struct hal_can_msg_t
 * @brief Representation of a CAN frame.
 */
typedef struct {
    uint32_t id;               /**< CAN identifier */
    hal_can_id_type_t id_type; /**< Standard or extended ID */
    hal_can_rtr_t rtr;         /**< Data or remote frame */
    uint8_t dlc;               /**< Data length code (0-8) */
    uint8_t data[8];           /**< Frame payload */
} hal_can_msg_t;

/**
 * @struct hal_can_config_t
 * @brief Configuration parameters for CAN peripherals.
 */
typedef struct {
    uint32_t baudrate; /**< CAN baud rate (e.g. 125000, 250000, 500000, 1000000) */
} hal_can_config_t;

/**
 * @brief Callback for CAN RX events.
 *
 * @param port   The CAN port that received a message.
 * @param msg    Pointer to the received message.
 * @param ctx    Caller-specified context pointer.
 */
typedef void (*hal_can_rx_cb_t)(hal_can_port_t port, const hal_can_msg_t *msg, void *ctx);

/**
 * @brief Callback for CAN TX complete events.
 *
 * @param port   The CAN port that finished transmitting.
 * @param ctx    Caller-specified context pointer.
 */
typedef void (*hal_can_tx_cb_t)(hal_can_port_t port, void *ctx);

/**
 * @brief Callback function type upon CAN errors.
 *
 * @param port   The CAN port where the error occurred.
 * @param error  The error code (hardware-specific or driver generic).
 * @param ctx    Caller-specified context pointer.
 */
typedef void (*hal_can_error_cb_t)(hal_can_port_t port, uint32_t error, void *ctx);

/** @} */ // end can_types

/**
 * @defgroup can_api Public API
 * @ingroup can
 * @brief External interface for interacting with the CAN driver.
 *
 * @{
 */

/**
 * @brief Initialize the CAN driver for a port.
 *
 * @param port    The CAN port to initialize.
 * @param config  Pointer to configuration structure.
 */
void hal_can_init(hal_can_port_t port, const hal_can_config_t *config);

/**
 * @brief Write (transmit) a message to the CAN bus.
 *
 * @param port  The CAN port.
 * @param msg   Pointer to the message to transmit.
 * @return hal_can_status_t Status code.
 */
hal_can_status_t hal_can_write(hal_can_port_t port, const hal_can_msg_t *msg);

/**
 * @brief Register a callback that fires when a new message is received.
 *
 * @param port  The CAN port.
 * @param cb    The RX callback function.
 * @param ctx   User-specified context pointer.
 */
void hal_can_register_rx_callback(hal_can_port_t port, hal_can_rx_cb_t cb, void *ctx);

/**
 * @brief Register a callback that fires when a TX operation is complete.
 *
 * @param port  The CAN port.
 * @param cb    The TX callback function.
 * @param ctx   User-specified context pointer.
 */
void hal_can_register_tx_callback(hal_can_port_t port, hal_can_tx_cb_t cb, void *ctx);

/**
 * @brief Register a callback that fires when a CAN error occurs.
 *
 * @param port  The CAN port.
 * @param cb    The error callback function.
 * @param ctx   User-specified context pointer.
 */
void hal_can_register_error_callback(hal_can_port_t port, hal_can_error_cb_t cb, void *ctx);

/**
 * @brief ISR entry point for CAN interrupts.
 * Should be called from the MCU's CAN IRQ handlers.
 *
 * @param port  The CAN port.
 */
void hal_can_isr_handler(hal_can_port_t port);

/** @} */ // end can_api
/** @} */ // end can

#endif // HAL_CAN_H
