/**
 * @file command_handler.h
 * @brief Application layer for processing incoming commands.
 */

#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include "osusat/event_bus.h"
#include "uart_events.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * @defgroup command_handler Command Handler Application
 * @brief Processes incoming commands and dispatches them to appropriate
 * services.
 *
 * This application is responsible for:
 *  - Receiving raw byte streams via UART events.
 *  - Parsing and validating incoming command packets.
 *  - Publishing events for other services to handle.
 *
 * @{
 */

/**
 * @defgroup command_handler_types Types
 * @ingroup command_handler
 * @brief Structures used by the Command Handler Application.
 *
 * @{
 */

/**
 * @struct command_handler_t
 * @brief State container for the command handler application.
 */
typedef struct {
    bool initialized;            /**< True if the application is initialized. */
    uart_events_t *primary_uart; /**< Pointer to primary UART. */
    uart_events_t *aux_uart;     /**< Pointer to auxiliary UART. */
    bool primary_uart_ok;        /**< Health status of primary UART. */

    // context for pending telemetry requests
    bool pending_reply;
    uint8_t pending_sequence;
    OSUSatDestination pending_destination;
    uint8_t pending_command_id;
} command_handler_t;

/** @} */ // end command_handler_types

/**
 * @defgroup command_handler_api Public API
 * @ingroup command_handler
 * @brief Functions for managing the command handler application.
 * @{
 */

/**
 * @brief Initialize the command handler application.
 *
 * @param[out] app The command handler application to initialize.
 * @param[in] primary_uart Primary UART events service instance.
 * @param[in] aux_uart Auxiliary UART events service instance.
 */
void command_handler_init(command_handler_t *app, uart_events_t *primary_uart,
                          uart_events_t *aux_uart);

/** @} */ // end command_handler_api

/** @} */ // end command_handler

#endif // COMMAND_HANDLER_H