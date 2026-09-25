/**
 * @file hal_time.h
 * @brief System Time & Millisecond/Microsecond Timer HAL for OBC.
 *
 * Provides functions for obtaining high-precision timestamps since boot.
 */

#ifndef HAL_TIME_H
#define HAL_TIME_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup obc_time Time Driver
 * @brief System time measurement functions.
 * @{
 */

/**
 * @brief Initialize the system time base counter.
 */
void hal_time_init(void);

/**
 * @brief Get elapsed time in milliseconds since system boot.
 *
 * @return Monotonic millisecond counter.
 */
uint32_t hal_time_get_ms(void);

/**
 * @brief Get elapsed time in microseconds since system boot.
 *
 * @return Monotonic microsecond counter.
 */
uint64_t hal_time_get_us(void);

/** @} */

#ifdef __cplusplus
}
#endif

#endif // HAL_TIME_H
