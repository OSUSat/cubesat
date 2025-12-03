/**
 * @file command_handler.h
 * @brief Application layer for processing incoming commands.
 */

#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

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
    bool initialized; /**< True if the application is initialized. */
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
 */
void command_handler_init(command_handler_t *app);

/** @} */ // end command_handler_api

/** @} */ // end command_handler

#endif // COMMAND_HANDLER_H