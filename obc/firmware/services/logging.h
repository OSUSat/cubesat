/**
 * @file logging.h
 * @brief OBC Logging Service
 *
 * Provides structured logging with buffering and flushing to a local mock FRAM.
 */

#ifndef LOGGING_H
#define LOGGING_H

#include "osusat/slog.h"
#include <stdint.h>
#include <stddef.h>

/**
 * @defgroup logging Logging Service
 * @brief Structured logging for OBC subsystems.
 *
 * @{
 */

/**
 * @defgroup logging_types Component IDs
 * @ingroup logging
 * @brief Component identifiers for log filtering and organization.
 *
 * @{
 */

#define OBC_COMPONENT_MAIN     0x20    /**< Main application */
#define OBC_COMPONENT_COMMAND  0x21    /**< Command handler */
#define OBC_COMPONENT_LOGGING  0x22    /**< Logging service */
#define OBC_COMPONENT_CAN      0x23    /**< CAN bus service */
#define OBC_COMPONENT_UART     0x24    /**< UART/Serial driver */
#define OBC_COMPONENT_SENSORS  0x25    /**< Sensor monitoring */
#define OBC_COMPONENT_WATCHDOG 0x26    /**< Watchdog management */

/** @} */ // end logging_types

/**
 * @defgroup logging_api Public API
 * @ingroup logging
 * @brief External interface for the logging service.
 *
 * @{
 */

#define FRAM_LOG_SIZE 16384  // 16KB mock FRAM log buffer

/**
 * @brief Initialize the logging service.
 *
 * @param[in] min_level Minimum log level to record.
 */
void logging_init(osusat_slog_level_t min_level);

/**
 * @brief Write an external log entry (e.g., from EPS) to mock FRAM and print it.
 *
 * @param[in] entry   Pointer to the log entry header.
 * @param[in] message Pointer to the log message string.
 */
void logging_write_external_log(const osusat_slog_entry_t *entry, const char *message);

/**
 * @brief Flush pending logs to the local mock FRAM.
 *
 * @return Number of log entries flushed.
 */
size_t logging_flush(void);

/**
 * @brief Change the minimum log level at runtime.
 *
 * @param[in] level New minimum log level.
 */
void logging_set_level(osusat_slog_level_t level);

/**
 * @brief Get number of pending log entries.
 *
 * @return Approximate count of buffered logs.
 */
size_t logging_pending_count(void);

/**
 * @brief Get a pointer to the mock FRAM buffer.
 *
 * @return Pointer to the FRAM log buffer.
 */
const uint8_t* logging_get_fram_buffer(void);

/**
 * @brief Get the current write pointer in the mock FRAM.
 *
 * @return Write pointer offset.
 */
uint32_t logging_get_fram_write_ptr(void);

/** @} */ // end logging_api

/** @} */ // end logging

#endif // LOGGING_H
