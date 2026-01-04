/**
 * @file hal_time_mock.c
 * @brief Mock time HAL for host testing.
 */

#include "hal_time.h"
#include <time.h>
#include <unistd.h>

static uint64_t g_start_time_us = 0;

/**
 * @brief Get current time in microseconds (Unix epoch)
 */
static uint64_t get_time_us(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000ULL + (uint64_t)ts.tv_nsec / 1000ULL;
}

void hal_time_init(void) { g_start_time_us = get_time_us(); }

uint32_t hal_time_get_ms(void) {
    if (g_start_time_us == 0) {
        g_start_time_us = get_time_us();
    }

    uint64_t elapsed_us = get_time_us() - g_start_time_us;
    return (uint32_t)(elapsed_us / 1000ULL);
}

uint64_t hal_time_get_us(void) {
    if (g_start_time_us == 0) {
        g_start_time_us = get_time_us();
    }

    return get_time_us() - g_start_time_us;
}

void hal_time_delay_ms(uint32_t ms) { usleep(ms * 1000); }
