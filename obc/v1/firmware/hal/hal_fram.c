/**
 * @file hal_fram.c
 * @brief FRAM memory hardware abstraction implementation for STM32H7.
 */

#include "hal_fram.h"
#include "hal_gpio.h"
#include "hal_i2c.h"
#include "obc_config.h"
#include <string.h>

#if defined(__arm__)
#include "stm32h7xx_hal.h"
#endif

void hal_fram_init(void) {
#if defined(__arm__)
    // drive WP low to enable writes to FRAM (WP is active high)
    hal_gpio_write(FRAM_WP_PIN_INDEX, HAL_GPIO_STATE_LOW);
#endif
}

hal_fram_status_t hal_fram_write(uint32_t address, const uint8_t *data,
                                 size_t size) {
    if (address + size > 2048) {
        return HAL_FRAM_ERROR; // out of bounds for 16Kb FRAM (FM24CL16B)
    }

    if (size == 0) {
        return HAL_FRAM_OK;
    }

#if defined(__arm__)
    // ensure WP is low (write enabled)
    hal_gpio_write(FRAM_WP_PIN_INDEX, HAL_GPIO_STATE_LOW);

    uint32_t bytes_written = 0;
    while (bytes_written < size) {
        uint32_t curr_addr = address + bytes_written;
        uint32_t page_index = (curr_addr >> 8) & 0x07;
        uint32_t offset_in_page = curr_addr & 0xFF;

        // calculate max bytes we can write in this page
        uint32_t max_write = 256 - offset_in_page;
        uint32_t chunk_size = (size - bytes_written < max_write)
                                  ? (size - bytes_written)
                                  : max_write;

        uint8_t i2c_addr = FRAM_I2C_ADDR | page_index;
        i2c_error_t err =
            hal_i2c_mem_write(FRAM_I2C_BUS, i2c_addr, (uint8_t)offset_in_page,
                              (uint8_t *)(data + bytes_written),
                              (uint16_t)chunk_size, NULL, NULL, NULL);

        if (err != I2C_HAL_ERR_NONE) {
            return HAL_FRAM_ERROR;
        }

        bytes_written += chunk_size;
    }

    return HAL_FRAM_OK;
#else
    (void)address;
    (void)data;

    return HAL_FRAM_OK;
#endif
}

hal_fram_status_t hal_fram_read(uint32_t address, uint8_t *buffer,
                                size_t size) {
    if (address + size > 2048) {
        return HAL_FRAM_ERROR; // Out of bounds
    }

    if (size == 0) {
        return HAL_FRAM_OK;
    }

#if defined(__arm__)
    uint32_t bytes_read = 0;
    while (bytes_read < size) {
        uint32_t curr_addr = address + bytes_read;
        uint32_t page_index = (curr_addr >> 8) & 0x07;
        uint32_t offset_in_page = curr_addr & 0xFF;

        // calculate max bytes we can read in this page
        uint32_t max_read = 256 - offset_in_page;
        uint32_t chunk_size =
            (size - bytes_read < max_read) ? (size - bytes_read) : max_read;

        uint8_t i2c_addr = FRAM_I2C_ADDR | page_index;
        i2c_error_t err = hal_i2c_mem_read(
            FRAM_I2C_BUS, i2c_addr, (uint8_t)offset_in_page,
            buffer + bytes_read, (uint16_t)chunk_size, NULL, NULL, NULL);

        if (err != I2C_HAL_ERR_NONE) {
            return HAL_FRAM_ERROR;
        }

        bytes_read += chunk_size;
    }

    return HAL_FRAM_OK;
#else
    (void)address;
    (void)buffer;

    return HAL_FRAM_OK;
#endif
}

#if defined(__arm__)
#define FRAM_QUEUE_SIZE 16

typedef struct {
    uint32_t address;
    uint8_t *buffer;
    size_t size;
    size_t bytes_transferred;
    hal_fram_callback_t cb;
    void *ctx;
    bool is_write;
} fram_req_t;

static fram_req_t g_fram_queue[FRAM_QUEUE_SIZE];
static volatile uint16_t g_queue_head = 0;
static volatile uint16_t g_queue_tail = 0;
static volatile bool g_fram_busy = false;

static fram_req_t g_active_req;

static void fram_process_queue(void);
static void fram_start_next_chunk(void);
static void fram_i2c_tx_callback(i2c_bus_t bus, void *ctx);
static void fram_i2c_rx_callback(i2c_bus_t bus, void *ctx);
static void fram_i2c_error_callback(i2c_bus_t bus, i2c_error_t err, void *ctx);

static bool queue_push(const fram_req_t *req) {
    uint16_t next = (g_queue_head + 1) % FRAM_QUEUE_SIZE;
    if (next == g_queue_tail) {
        return false; // Queue full
    }
    g_fram_queue[g_queue_head] = *req;
    g_queue_head = next;
    return true;
}

static bool queue_pop(fram_req_t *req) {
    if (g_queue_head == g_queue_tail) {
        return false; // Queue empty
    }
    *req = g_fram_queue[g_queue_tail];
    g_queue_tail = (g_queue_tail + 1) % FRAM_QUEUE_SIZE;
    return true;
}

static void fram_process_queue(void) {
    uint32_t primask = __get_PRIMASK();
    __asm volatile("cpsid i" : : : "memory");
    
    if (g_fram_busy) {
        __set_PRIMASK(primask);
        __asm volatile("" ::: "memory");
        return;
    }
    
    if (!queue_pop(&g_active_req)) {
        __set_PRIMASK(primask);
        __asm volatile("" ::: "memory");
        return;
    }
    
    g_fram_busy = true;
    
    __set_PRIMASK(primask);
    __asm volatile("" ::: "memory");
    
    fram_start_next_chunk();
}

static void fram_start_next_chunk(void) {
    uint32_t curr_addr = g_active_req.address + g_active_req.bytes_transferred;
    size_t remaining = g_active_req.size - g_active_req.bytes_transferred;
    
    if (remaining == 0) {
        g_fram_busy = false;
        if (g_active_req.cb) {
            g_active_req.cb(HAL_FRAM_OK, g_active_req.ctx);
        }
        fram_process_queue();
        return;
    }
    
    uint32_t page_index = (curr_addr >> 8) & 0x07;
    uint32_t offset_in_page = curr_addr & 0xFF;
    uint32_t max_chunk = 256 - offset_in_page;
    uint32_t chunk_size = (remaining < max_chunk) ? remaining : max_chunk;
    uint8_t i2c_addr = FRAM_I2C_ADDR | page_index;
    
    i2c_error_t err;
    if (g_active_req.is_write) {
        // Ensure WP is low (write enabled)
        hal_gpio_write(FRAM_WP_PIN_INDEX, HAL_GPIO_STATE_LOW);
        
        err = hal_i2c_mem_write(
            FRAM_I2C_BUS, i2c_addr, (uint8_t)offset_in_page,
            g_active_req.buffer + g_active_req.bytes_transferred,
            (uint16_t)chunk_size, fram_i2c_tx_callback, fram_i2c_error_callback, NULL);
    } else {
        err = hal_i2c_mem_read(
            FRAM_I2C_BUS, i2c_addr, (uint8_t)offset_in_page,
            g_active_req.buffer + g_active_req.bytes_transferred,
            (uint16_t)chunk_size, fram_i2c_rx_callback, fram_i2c_error_callback, NULL);
    }
    
    if (err != I2C_HAL_ERR_NONE) {
        g_fram_busy = false;
        if (g_active_req.cb) {
            g_active_req.cb(HAL_FRAM_ERROR, g_active_req.ctx);
        }
        fram_process_queue();
    }
}

static void fram_i2c_tx_callback(i2c_bus_t bus, void *ctx) {
    (void)bus;
    (void)ctx;
    
    uint32_t curr_addr = g_active_req.address + g_active_req.bytes_transferred;
    uint32_t offset_in_page = curr_addr & 0xFF;
    uint32_t max_chunk = 256 - offset_in_page;
    size_t remaining = g_active_req.size - g_active_req.bytes_transferred;
    uint32_t chunk_size = (remaining < max_chunk) ? remaining : max_chunk;
    
    g_active_req.bytes_transferred += chunk_size;
    fram_start_next_chunk();
}

static void fram_i2c_rx_callback(i2c_bus_t bus, void *ctx) {
    (void)bus;
    (void)ctx;
    
    uint32_t curr_addr = g_active_req.address + g_active_req.bytes_transferred;
    uint32_t offset_in_page = curr_addr & 0xFF;
    uint32_t max_chunk = 256 - offset_in_page;
    size_t remaining = g_active_req.size - g_active_req.bytes_transferred;
    uint32_t chunk_size = (remaining < max_chunk) ? remaining : max_chunk;
    
    g_active_req.bytes_transferred += chunk_size;
    fram_start_next_chunk();
}

static void fram_i2c_error_callback(i2c_bus_t bus, i2c_error_t err, void *ctx) {
    (void)bus;
    (void)err;
    (void)ctx;
    
    g_fram_busy = false;
    if (g_active_req.cb) {
        g_active_req.cb(HAL_FRAM_ERROR, g_active_req.ctx);
    }
    fram_process_queue();
}
#endif

hal_fram_status_t hal_fram_write_it(uint32_t address, const uint8_t *data,
                                    size_t size, hal_fram_callback_t cb, void *ctx) {
    if (address + size > 2048) {
        return HAL_FRAM_ERROR;
    }
    if (size == 0) {
        if (cb) cb(HAL_FRAM_OK, ctx);
        return HAL_FRAM_OK;
    }
    
#if defined(__arm__)
    fram_req_t req = {
        .address = address,
        .buffer = (uint8_t *)data, // cast away const for storage in queue
        .size = size,
        .bytes_transferred = 0,
        .cb = cb,
        .ctx = ctx,
        .is_write = true
    };
    
    uint32_t primask = __get_PRIMASK();
    __asm volatile("cpsid i" : : : "memory");
    
    bool ok = queue_push(&req);
    
    __set_PRIMASK(primask);
    __asm volatile("" ::: "memory");
    
    if (!ok) {
        return HAL_FRAM_ERROR;
    }
    
    fram_process_queue();
    return HAL_FRAM_OK;
#else
    (void)address;
    (void)data;
    if (cb) cb(HAL_FRAM_OK, ctx);
    return HAL_FRAM_OK;
#endif
}

hal_fram_status_t hal_fram_read_it(uint32_t address, uint8_t *buffer,
                                   size_t size, hal_fram_callback_t cb, void *ctx) {
    if (address + size > 2048) {
        return HAL_FRAM_ERROR;
    }
    if (size == 0) {
        if (cb) cb(HAL_FRAM_OK, ctx);
        return HAL_FRAM_OK;
    }
    
#if defined(__arm__)
    fram_req_t req = {
        .address = address,
        .buffer = buffer,
        .size = size,
        .bytes_transferred = 0,
        .cb = cb,
        .ctx = ctx,
        .is_write = false
    };
    
    uint32_t primask = __get_PRIMASK();
    __asm volatile("cpsid i" : : : "memory");
    
    bool ok = queue_push(&req);
    
    __set_PRIMASK(primask);
    __asm volatile("" ::: "memory");
    
    if (!ok) {
        return HAL_FRAM_ERROR;
    }
    
    fram_process_queue();
    return HAL_FRAM_OK;
#else
    (void)address;
    (void)buffer;
    if (cb) cb(HAL_FRAM_OK, ctx);
    return HAL_FRAM_OK;
#endif
}
