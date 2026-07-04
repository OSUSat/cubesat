/**
 * @file hal_can_mock.h
 * @brief Mock utilities for the CAN HAL.
 *
 * This module provides additional functions used exclusively during testing.
 * These allow injecting fake CAN messages, inspecting transmitted messages,
 * and resetting the internal mock state.
 */

#ifndef HAL_CAN_MOCK_H
#define HAL_CAN_MOCK_H

#include "hal_can.h"
#include <stddef.h>

/**
 * @defgroup can_mock CAN Mock
 * @ingroup can
 * @brief Testing utilities for the CAN module.
 *
 * @{
 */

/**
 * @brief Inject a CAN message into the RX callback queue.
 *
 * Simulates a message arriving from the CAN bus.
 *
 * @param port  The CAN port.
 * @param msg   Pointer to the message to inject.
 */
void mock_can_push_rx(hal_can_port_t port, const hal_can_msg_t *msg);

/**
 * @brief Retrieve a message that was transmitted.
 *
 * @param port     The CAN port.
 * @param out      Buffer to copy the message to.
 * @param max_len  Maximum number of messages to retrieve.
 * @return size_t  Number of messages retrieved.
 */
size_t mock_can_get_tx(hal_can_port_t port, hal_can_msg_t *out, size_t max_len);

/**
 * @brief Reset the CAN mock state (clear all queues).
 */
void mock_can_reset(void);

/** @} */ // end can_mock

#endif // HAL_CAN_MOCK_H
