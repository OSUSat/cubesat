/**
 * @file hal_time.h
 * @brief Time hardware abstraction public API.
 *
 * Provides platform-independent time measurement functionality.
 */

#ifndef HAL_TIME_H
#define HAL_TIME_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup hal_time Time HAL
 * @brief Platform-independent time measurement.
 *
 * @{
 */

/**
 * @defgroup hal_time_api Public API
 * @ingroup hal_time
 * @brief External interface for time measurement.
 *
 * @{
 */

/**
 * @brief Initialize the time HAL.
 *
 * Must be called during system initialization.
 */
void hal_time_init(void);

/**
 * @brief Get current system time in milliseconds.
 *
 * @return Current time in milliseconds since system start.
 */
uint32_t hal_time_get_ms(void);

/**
 * @brief Get current system time in microseconds.
 *
 * @return Current time in microseconds since system start.
 *
 * @note May not be available on all platforms. Returns ms * 1000 if
 * unavailable.
 */
uint64_t hal_time_get_us(void);

/**
 * @brief Blocking delay in milliseconds.
 *
 * @param[in] ms Number of milliseconds to delay.
 */
void hal_time_delay_ms(uint32_t ms);

/** @} */ // end hal_time_api

/** @} */ // end hal_time

#ifdef __cplusplus
}
#endif

#endif // HAL_TIME_H
