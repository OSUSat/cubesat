/**
 * @file logging.c
 * @brief OBC Logging Service Implementation
 */

#include "logging.h"
#include "hal_time.h"
#include "hal_uart.h"
#include "osusat/event_bus.h"
#include "osusat/ring_buffer.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define LOG_STORAGE_SIZE 4096
static uint8_t log_storage[LOG_STORAGE_SIZE];
static osusat_ring_buffer_t log_ring_buffer;
static uint32_t tick_counter = 0;
#define LOG_FLUSH_INTERVAL_CYCLES 100

// mock FRAM Buffer
static uint8_t fram_mock_buffer[FRAM_LOG_SIZE];
static uint32_t g_fram_log_write_ptr = 0;

static void logging_handle_tick(const osusat_event_t *e, void *ctx);
static void write_to_fram(const uint8_t *data, size_t size);
static const char *slog_level_str(uint8_t level);

static void semihosting_write_str(const char *str, size_t len) {
#if defined(__arm__)
    // SYS_WRITE (0x05)
    // r0 = 0x05
    // r1 = pointer to 3-word argument block:
    //      word 0: file handle (1 for stdout)
    //      word 1: pointer to string
    //      word 2: length of string
    uint32_t args[3];
    args[0] = 1; // stdout
    args[1] = (uint32_t)str;
    args[2] = (uint32_t)len;
    __asm volatile("mov r0, #5\n"
                   "mov r1, %0\n"
                   "bkpt 0xAB\n"
                   :
                   : "r"(args)
                   : "r0", "r1", "memory");
#else
    (void)str;
    (void)len;
#endif
}

/**
 * @brief Strong override of syscalls.c _write to redirect all printf/stdout.
 * Falls back to physical hardware UART when SEMIHOSTING is not explicitly
 * compiled in.
 */
int _write(int file, char *ptr, int len) {
    (void)file;
#if defined(__arm__)
#if defined(USE_SEMIHOSTING)
    semihosting_write_str(ptr, len);
#else
    hal_uart_write(UART_PORT_1, (uint8_t *)ptr, len);
#endif
#else
    fwrite(ptr, 1, len, stdout);
    fflush(stdout);
#endif
    return len;
}

int __io_putchar(int ch) {
    char c = (char)ch;
    _write(1, &c, 1);
    return ch;
}

void logging_init(osusat_slog_level_t min_level) {
    // clear mock FRAM
    memset(fram_mock_buffer, 0, sizeof(fram_mock_buffer));
    g_fram_log_write_ptr = 0;

    // initialize the ring buffer used by slog
    osusat_ring_buffer_init(&log_ring_buffer, log_storage, sizeof(log_storage),
                            true);

    // initialize osusat structured logging library
    osusat_slog_init(&log_ring_buffer, hal_time_get_ms, min_level);

    // subscribe to Systick events for periodic flushing
    osusat_event_bus_subscribe(EVENT_SYSTICK, logging_handle_tick, NULL);

    LOG_INFO(OBC_COMPONENT_LOGGING,
             "OBC Logging service initialized (Mock FRAM size: %d bytes)",
             FRAM_LOG_SIZE);
}

void logging_write_external_log(const osusat_slog_entry_t *entry,
                                const char *message) {
    // write the structured log to mock FRAM
    size_t msg_len_with_null = entry->message_len + 1;
    write_to_fram((const uint8_t *)entry, sizeof(*entry));
    write_to_fram((const uint8_t *)message, msg_len_with_null);

    // also print to debug console (semihosting via printf)
    printf("[EPS] [%6lu] [%s] [Comp: 0x%02X] [Line: %u] %s\r\n",
           (unsigned long)entry->timestamp_ms, slog_level_str(entry->level),
           entry->component_id, entry->line, message);
}

static void write_to_fram(const uint8_t *data, size_t size) {
    if (g_fram_log_write_ptr + size > FRAM_LOG_SIZE) {
        g_fram_log_write_ptr = 0; // Wrap around to the beginning
    }

    memcpy(&fram_mock_buffer[g_fram_log_write_ptr], data, size);
    g_fram_log_write_ptr += size;
}

static const char *slog_level_str(uint8_t level) {
    switch (level) {
    case OSUSAT_SLOG_DEBUG:
        return "DEBUG";
    case OSUSAT_SLOG_INFO:
        return "INFO";
    case OSUSAT_SLOG_WARN:
        return "WARN";
    case OSUSAT_SLOG_ERROR:
        return "ERROR";
    case OSUSAT_SLOG_CRITICAL:
        return "CRITICAL";
    default:
        return "UNKNOWN";
    }
}

static void log_flush_callback(const osusat_slog_entry_t *entry,
                               const char *message, void *user_ctx) {
    (void)user_ctx;

    // write the structured log to mock FRAM
    size_t msg_len_with_null = entry->message_len + 1;
    write_to_fram((const uint8_t *)entry, sizeof(*entry));
    write_to_fram((const uint8_t *)message, msg_len_with_null);

    // also print to debug console (UART)
    printf("[%6lu] [%s] [Comp: 0x%02X] [Line: %u] %s\r\n",
           (unsigned long)entry->timestamp_ms, slog_level_str(entry->level),
           entry->component_id, entry->line, message);
}

size_t logging_flush(void) {
    if (osusat_slog_pending_count() == 0) {
        return 0;
    }

    return osusat_slog_flush(log_flush_callback, NULL);
}

void logging_set_level(osusat_slog_level_t level) {
    osusat_slog_change_min_log_level(level);
    LOG_INFO(OBC_COMPONENT_LOGGING, "Min log level changed to %d", level);
}

size_t logging_pending_count(void) { return osusat_slog_pending_count(); }

const uint8_t *logging_get_fram_buffer(void) { return fram_mock_buffer; }

uint32_t logging_get_fram_write_ptr(void) { return g_fram_log_write_ptr; }

static void logging_handle_tick(const osusat_event_t *e, void *ctx) {
    (void)ctx;
    (void)e;

    tick_counter++;

    if (tick_counter >= LOG_FLUSH_INTERVAL_CYCLES) {
        tick_counter = 0;
        logging_flush();
    }
}
