/**
 * @file logging.h
 * @brief EPS Logging Service
 *
 * Provides structured logging with automatic flushing to OBC via UART.
 */

#ifndef LOGGING_H
#define LOGGING_H

#include "osusat/slog.h"
#include "uart_events.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup logging Logging Service
 * @brief Structured logging for EPS subsystems.
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

#define EPS_COMPONENT_MAIN 0x10    /**< Main application */
#define EPS_COMPONENT_RAIL 0x11    /**< Rail controller */
#define EPS_COMPONENT_MPPT 0x12    /**< MPPT controller */
#define EPS_COMPONENT_POWER 0x13   /**< Power policies */
#define EPS_COMPONENT_CMD 0x14     /**< Command handler */
#define EPS_COMPONENT_COMMS 0x15   /**< Communications */
#define EPS_COMPONENT_ADC 0x16     /**< ADC/sensing */
#define EPS_COMPONENT_PROFILE 0x17 /**< Power profiles */

/** @} */ // end logging_types

/**
 * @defgroup logging_api Public API
 * @ingroup logging
 * @brief External interface for the logging service.
 *
 * @{
 */

/**
 * @brief Initialize the logging service.
 *
 * Must be called after hal_uart_init() and before any logging.
 *
 * @param[in] min_level Minimum log level to record.
 */
void logging_init(osusat_slog_level_t min_level, uart_events_t *primary_uart,
                  uart_events_t *aux_uart);

/**
 * @brief Flush pending logs to OBC via UART.
 *
 * Should be called periodically from main loop or triggered by events.
 *
 * @return Number of log entries flushed.
 */
size_t logging_flush(void);

/**
 * @brief Change the minimum log level at runtime.
 *
 * Useful for debugging, can be called via command handler.
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

/** @} */ // end logging_api

/** @} */ // end logging

#ifdef __cplusplus
}
#endif

#endif // LOGGING_H
