/**
 * @file sensors.c
 * @brief Non-blocking barometric pressure and temperature sensor reading service implementation.
 */

#include "sensors.h"
#include "logging.h"
#include "hal_i2c.h"
#include "hal_time.h"
#include "osusat/event_bus.h"
#include "hal_fram.h"
#include "hal_flash.h"
#include "hal_gpio.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#if defined(__arm__)
#include "stm32h7xx_hal.h"
#define delay_ms(ms) HAL_Delay(ms)
#else
#define delay_ms(ms) (void)(ms)
#endif

// MS5607-02BA barometric pressure sensor I2C address and commands
#define MS5607_I2C_ADDR         0x77
#define MS5607_CMD_RESET        0x1E
#define MS5607_CMD_PROM_READ    0xA0
#define MS5607_CMD_CONV_D1_4096 0x48  // Pressure conversion, OSR=4096
#define MS5607_CMD_CONV_D2_4096 0x58  // Temperature conversion, OSR=4096
#define MS5607_CMD_ADC_READ     0x00

typedef enum {
    SENSOR_STATE_UNINITIALIZED = 0,
    SENSOR_STATE_RESETTING,
    SENSOR_STATE_READING_PROM_C1,
    SENSOR_STATE_READING_PROM_C2,
    SENSOR_STATE_READING_PROM_C3,
    SENSOR_STATE_READING_PROM_C4,
    SENSOR_STATE_READING_PROM_C5,
    SENSOR_STATE_READING_PROM_C6,
    SENSOR_STATE_IDLE,
    SENSOR_STATE_CONVERTING_D1,
    SENSOR_STATE_READING_D1,
    SENSOR_STATE_CONVERTING_D2,
    SENSOR_STATE_READING_D2
} sensor_state_t;

// Calibration coefficients from PROM
static uint16_t C1 = 0; // Pressure sensitivity (SENS_T1)
static uint16_t C2 = 0; // Pressure offset (OFF_T1)
static uint16_t C3 = 0; // Temperature coefficient of pressure sensitivity (TCS)
static uint16_t C4 = 0; // Temperature coefficient of pressure offset (TCO)
static uint16_t C5 = 0; // Reference temperature (TREF)
static uint16_t C6 = 0; // Temperature coefficient of the temperature (TEMPSENS)

static volatile bool prom_read_success = false;

// State machine variables
static volatile sensor_state_t current_state = SENSOR_STATE_UNINITIALIZED;
static uint32_t state_start_time = 0;
static uint32_t last_read_time = 0;

static uint32_t raw_pressure = 0;
static uint32_t raw_temperature = 0;

// Persistent/Asynchronous packed telemetry recording state
#define TELEMETRY_FRAM_START_ADDR 1024
#define TELEMETRY_FRAM_MAX_SAMPLES 124 // (1024 - 32 header) / 8 bytes per sample

typedef struct __attribute__((packed)) {
    uint32_t magic;             // Magic number (0x54454C4D)
    uint32_t sequence_number;   // Sequence number, incremented on each block write
    uint32_t timestamp_ms;      // Timestamp (ms) when this block was started
    uint32_t flash_write_addr;  // Next flash address to write
    uint32_t sample_count;      // Number of samples currently in the block
    uint8_t  reserved[12];      // Pad to 32 bytes
} telemetry_block_header_t;

typedef struct __attribute__((packed)) {
    uint32_t timestamp_ms;
    int16_t  temp_centi_c;
    uint16_t pres_mbar;
} telemetry_sample_t;

static uint32_t g_sequence_number = 0;
static uint32_t g_telemetry_index = 0;
static uint32_t g_flash_write_addr = 0x08080000;

// Simulation variables
static int32_t g_sim_ascent_rate = 0;
static int32_t g_sim_ascent_offset = 0;
static bool g_sim_launch_triggered = false;

static void sensors_handle_tick(const osusat_event_t *e, void *ctx);
static void sensors_process_data(void);

#if !defined(__arm__)
static void sensors_read_and_process_mock(void);
#endif

// Buffer and command variables for async transfer (must remain valid during transfer)
#if defined(__arm__)
static uint8_t g_prom_cmd;
static uint8_t g_prom_buf[2];
static uint8_t g_adc_cmd;
static uint8_t g_adc_buf[3];
static uint8_t g_reset_cmd;
static uint8_t g_conv_cmd;

static void ms5607_tx_callback(i2c_bus_t bus, void *ctx);
static void ms5607_rx_callback(i2c_bus_t bus, void *ctx);
static void ms5607_error_callback(i2c_bus_t bus, i2c_error_t err, void *ctx);

static void ms5607_tx_callback(i2c_bus_t bus, void *ctx) {
    (void)bus;
    (void)ctx;

    switch (current_state) {
        case SENSOR_STATE_READING_PROM_C1:
        case SENSOR_STATE_READING_PROM_C2:
        case SENSOR_STATE_READING_PROM_C3:
        case SENSOR_STATE_READING_PROM_C4:
        case SENSOR_STATE_READING_PROM_C5:
        case SENSOR_STATE_READING_PROM_C6:
            hal_i2c_read(I2C_BUS_2, MS5607_I2C_ADDR, g_prom_buf, 2,
                         ms5607_rx_callback, ms5607_error_callback, NULL);
            break;

        case SENSOR_STATE_READING_D1:
        case SENSOR_STATE_READING_D2:
            hal_i2c_read(I2C_BUS_2, MS5607_I2C_ADDR, g_adc_buf, 3,
                         ms5607_rx_callback, ms5607_error_callback, NULL);
            break;

        default:
            break;
    }
}

static void ms5607_rx_callback(i2c_bus_t bus, void *ctx) {
    (void)bus;
    (void)ctx;

    uint32_t now = hal_time_get_ms();

    switch (current_state) {
        case SENSOR_STATE_READING_PROM_C1:
            C1 = ((uint16_t)g_prom_buf[0] << 8) | g_prom_buf[1];
            g_prom_cmd = MS5607_CMD_PROM_READ + 4; // C2 command
            if (hal_i2c_write(I2C_BUS_2, MS5607_I2C_ADDR, &g_prom_cmd, 1,
                              ms5607_tx_callback, ms5607_error_callback, NULL) == I2C_HAL_ERR_NONE) {
                current_state = SENSOR_STATE_READING_PROM_C2;
            }
            break;

        case SENSOR_STATE_READING_PROM_C2:
            C2 = ((uint16_t)g_prom_buf[0] << 8) | g_prom_buf[1];
            g_prom_cmd = MS5607_CMD_PROM_READ + 6; // C3 command
            if (hal_i2c_write(I2C_BUS_2, MS5607_I2C_ADDR, &g_prom_cmd, 1,
                              ms5607_tx_callback, ms5607_error_callback, NULL) == I2C_HAL_ERR_NONE) {
                current_state = SENSOR_STATE_READING_PROM_C3;
            }
            break;

        case SENSOR_STATE_READING_PROM_C3:
            C3 = ((uint16_t)g_prom_buf[0] << 8) | g_prom_buf[1];
            g_prom_cmd = MS5607_CMD_PROM_READ + 8; // C4 command
            if (hal_i2c_write(I2C_BUS_2, MS5607_I2C_ADDR, &g_prom_cmd, 1,
                              ms5607_tx_callback, ms5607_error_callback, NULL) == I2C_HAL_ERR_NONE) {
                current_state = SENSOR_STATE_READING_PROM_C4;
            }
            break;

        case SENSOR_STATE_READING_PROM_C4:
            C4 = ((uint16_t)g_prom_buf[0] << 8) | g_prom_buf[1];
            g_prom_cmd = MS5607_CMD_PROM_READ + 10; // C5 command
            if (hal_i2c_write(I2C_BUS_2, MS5607_I2C_ADDR, &g_prom_cmd, 1,
                              ms5607_tx_callback, ms5607_error_callback, NULL) == I2C_HAL_ERR_NONE) {
                current_state = SENSOR_STATE_READING_PROM_C5;
            }
            break;

        case SENSOR_STATE_READING_PROM_C5:
            C5 = ((uint16_t)g_prom_buf[0] << 8) | g_prom_buf[1];
            g_prom_cmd = MS5607_CMD_PROM_READ + 12; // C6 command
            if (hal_i2c_write(I2C_BUS_2, MS5607_I2C_ADDR, &g_prom_cmd, 1,
                              ms5607_tx_callback, ms5607_error_callback, NULL) == I2C_HAL_ERR_NONE) {
                current_state = SENSOR_STATE_READING_PROM_C6;
            }
            break;

        case SENSOR_STATE_READING_PROM_C6:
            C6 = ((uint16_t)g_prom_buf[0] << 8) | g_prom_buf[1];
            LOG_INFO(OBC_COMPONENT_SENSORS,
                     "MS5607-02BA Initialized. C1=%u C2=%u C3=%u C4=%u C5=%u C6=%u",
                     C1, C2, C3, C4, C5, C6);
            prom_read_success = true;
            current_state = SENSOR_STATE_IDLE;
            last_read_time = now;
            break;

        case SENSOR_STATE_READING_D1:
            raw_pressure = ((uint32_t)g_adc_buf[0] << 16) | ((uint32_t)g_adc_buf[1] << 8) | g_adc_buf[2];
            g_conv_cmd = MS5607_CMD_CONV_D2_4096;
            if (hal_i2c_write(I2C_BUS_2, MS5607_I2C_ADDR, &g_conv_cmd, 1,
                              ms5607_tx_callback, ms5607_error_callback, NULL) == I2C_HAL_ERR_NONE) {
                current_state = SENSOR_STATE_CONVERTING_D2;
                state_start_time = now;
            }
            break;

        case SENSOR_STATE_READING_D2:
            raw_temperature = ((uint32_t)g_adc_buf[0] << 16) | ((uint32_t)g_adc_buf[1] << 8) | g_adc_buf[2];
            sensors_process_data();
            current_state = SENSOR_STATE_IDLE;
            break;

        default:
            break;
    }
}

static void ms5607_error_callback(i2c_bus_t bus, i2c_error_t err, void *ctx) {
    (void)bus;
    (void)ctx;
    LOG_ERROR(OBC_COMPONENT_SENSORS, "MS5607 I2C Error occurred! Code: %d. Resetting state.", err);
    current_state = SENSOR_STATE_UNINITIALIZED;
    prom_read_success = false;
    last_read_time = hal_time_get_ms(); // retry in 1s
}
#endif

void sensors_init(void) {
#if !defined(__arm__)
    // Mock values for host testing
    C1 = 40127;
    C2 = 36924;
    C3 = 23317;
    C4 = 23282;
    C5 = 33464;
    C6 = 28312;
    prom_read_success = true;
    LOG_INFO(OBC_COMPONENT_SENSORS, "MS5607-02BA Mock Initialized (Host)");
    current_state = SENSOR_STATE_IDLE;
#else
    current_state = SENSOR_STATE_UNINITIALIZED;
    prom_read_success = false;
#endif

    state_start_time = 0;
    last_read_time = 0;

    // Recover telemetry state from F-RAM
    telemetry_block_header_t header;
    if (hal_fram_read(TELEMETRY_FRAM_START_ADDR, (uint8_t *)&header, sizeof(header)) == HAL_FRAM_OK &&
        header.magic == 0x54454C4D &&
        header.flash_write_addr >= 0x08080000 &&
        header.flash_write_addr < 0x08100000 &&
        header.sample_count <= TELEMETRY_FRAM_MAX_SAMPLES) {
        
        g_flash_write_addr = header.flash_write_addr;
        g_sequence_number = header.sequence_number;
        g_telemetry_index = header.sample_count;
        
        LOG_INFO(OBC_COMPONENT_SENSORS, 
                 "Recovered telemetry state from F-RAM: seq=%lu, flash_addr=0x%08lX, samples=%lu",
                 (unsigned long)g_sequence_number, (unsigned long)g_flash_write_addr, (unsigned long)g_telemetry_index);
    } else {
        g_flash_write_addr = 0x08080000;
        g_sequence_number = 0;
        g_telemetry_index = 0;
        
        telemetry_block_header_t new_header = {
            .magic = 0x54454C4D,
            .sequence_number = 0,
            .timestamp_ms = hal_time_get_ms(),
            .flash_write_addr = 0x08080000,
            .sample_count = 0
        };
        memset(new_header.reserved, 0, sizeof(new_header.reserved));
        
        hal_fram_write(TELEMETRY_FRAM_START_ADDR, (const uint8_t *)&new_header, sizeof(new_header));
        
        LOG_INFO(OBC_COMPONENT_SENSORS, 
                 "Telemetry state uninitialized or corrupt. Initialized new block at 0x08080000");
    }

    // Subscribe to Systick for periodic non-blocking sampling
    osusat_event_bus_subscribe(EVENT_SYSTICK, sensors_handle_tick, NULL);
}

static void sensors_handle_tick(const osusat_event_t *e, void *ctx) {
    (void)e;
    (void)ctx;

    uint32_t now = hal_time_get_ms();

#if defined(__arm__)
    // PC13 Button is active low (normally HIGH, LOW when pressed)
    if (hal_gpio_read(2) == HAL_GPIO_STATE_LOW) {
        if (!g_sim_launch_triggered) {
            g_sim_launch_triggered = true;
            g_sim_ascent_rate = 5; // Drop pressure by 5 mbar/sec
            LOG_WARN(OBC_COMPONENT_SENSORS, "BUTTON1 pressed! Launch simulation triggered.");
        }
    }

    static uint32_t last_sim_tick = 0;
    if (g_sim_launch_triggered && (now - last_sim_tick >= 1000)) {
        last_sim_tick = now;
        g_sim_ascent_offset += g_sim_ascent_rate;
    }
#endif

#if !defined(__arm__)
    if (current_state == SENSOR_STATE_IDLE) {
        if (now - last_read_time >= 1000) {
            sensors_read_and_process_mock();
            last_read_time = now;
        }
    }
#else
    switch (current_state) {
        case SENSOR_STATE_UNINITIALIZED: {
            g_reset_cmd = MS5607_CMD_RESET;
            i2c_error_t err = hal_i2c_write(I2C_BUS_2, MS5607_I2C_ADDR, &g_reset_cmd, 1,
                                            ms5607_tx_callback, ms5607_error_callback, NULL);
            if (err == I2C_HAL_ERR_NONE) {
                current_state = SENSOR_STATE_RESETTING;
                state_start_time = now;
            } else {
                static uint32_t last_log_time = 0;
                if (now - last_log_time >= 1000) {
                    last_log_time = now;
                    LOG_ERROR(OBC_COMPONENT_SENSORS, "Failed to start reset write! Err: %d", err);
                }
                state_start_time = now;
            }
            break;
        }

        case SENSOR_STATE_RESETTING:
            // Wait 10 ms for reset reload
            if (now - state_start_time >= 10) {
                g_prom_cmd = MS5607_CMD_PROM_READ + 2;
                i2c_error_t err = hal_i2c_write(I2C_BUS_2, MS5607_I2C_ADDR, &g_prom_cmd, 1,
                                                ms5607_tx_callback, ms5607_error_callback, NULL);
                if (err == I2C_HAL_ERR_NONE) {
                    current_state = SENSOR_STATE_READING_PROM_C1;
                }
            }
            break;

        case SENSOR_STATE_READING_PROM_C1:
        case SENSOR_STATE_READING_PROM_C2:
        case SENSOR_STATE_READING_PROM_C3:
        case SENSOR_STATE_READING_PROM_C4:
        case SENSOR_STATE_READING_PROM_C5:
        case SENSOR_STATE_READING_PROM_C6:
            // Managed by I2C completion callbacks
            break;

        case SENSOR_STATE_IDLE:
            // Read every 1000 ms (1 second)
            if (now - last_read_time >= 1000) {
                g_conv_cmd = MS5607_CMD_CONV_D1_4096;
                i2c_error_t err = hal_i2c_write(I2C_BUS_2, MS5607_I2C_ADDR, &g_conv_cmd, 1,
                                                ms5607_tx_callback, ms5607_error_callback, NULL);
                if (err == I2C_HAL_ERR_NONE) {
                    current_state = SENSOR_STATE_CONVERTING_D1;
                    state_start_time = now;
                    last_read_time = now; // Set immediately to prevent timing drift
                }
            }
            break;

        case SENSOR_STATE_CONVERTING_D1:
            // Wait 15 ms to ensure conversion completes (9.04ms max OSR 4096)
            if (now - state_start_time >= 15) {
                g_adc_cmd = MS5607_CMD_ADC_READ;
                i2c_error_t err = hal_i2c_write(I2C_BUS_2, MS5607_I2C_ADDR, &g_adc_cmd, 1,
                                                ms5607_tx_callback, ms5607_error_callback, NULL);
                if (err == I2C_HAL_ERR_NONE) {
                    current_state = SENSOR_STATE_READING_D1;
                }
            }
            break;

        case SENSOR_STATE_READING_D1:
            break;

        case SENSOR_STATE_CONVERTING_D2:
            // Wait 15 ms
            if (now - state_start_time >= 15) {
                g_adc_cmd = MS5607_CMD_ADC_READ;
                i2c_error_t err = hal_i2c_write(I2C_BUS_2, MS5607_I2C_ADDR, &g_adc_cmd, 1,
                                                ms5607_tx_callback, ms5607_error_callback, NULL);
                if (err == I2C_HAL_ERR_NONE) {
                    current_state = SENSOR_STATE_READING_D2;
                }
            }
            break;

        case SENSOR_STATE_READING_D2:
            break;
    }
#endif
}

#if !defined(__arm__)
static void sensors_read_and_process_mock(void) {
    static uint32_t mock_step = 0;
    mock_step++;
    
    // Simulate raw values from datasheet example with slight variations
    raw_pressure = 6465444 + (mock_step % 100) * 10;
    raw_temperature = 8077636 + (mock_step % 50) * 5;
    
    sensors_process_data();
}
#endif

static void sensors_process_data(void) {
    int64_t D1 = raw_pressure;
    int64_t D2 = raw_temperature;

    // Compensation math (64-bit integers)
    int64_t dT = D2 - ((int64_t)C5 * 256);
    int64_t temp = 2000 + ((dT * C6) / 8388608);

    int64_t off = ((int64_t)C2 * 131072) + (((int64_t)C4 * dT) / 64);
    int64_t sens = ((int64_t)C1 * 65536) + (((int64_t)C3 * dT) / 128);

    int64_t pressure_pa = (((D1 * sens) / 2097152) - off) / 32768;
    int64_t pressure_mbar = (pressure_pa / 100) - g_sim_ascent_offset;

    // Publish data
    sensor_data_t sensor_data;
    sensor_data.barometer.pressure = pressure_mbar;
    sensor_data.barometer.temperature = temp;

    osusat_event_bus_publish(SENSORS_EVENT_DATA_RECEIVED, &sensor_data, sizeof(sensor_data));

    // Log to FRAM via logging service
    LOG_INFO(OBC_COMPONENT_SENSORS, 
             "Sensor telemetry -> Temp: %ld.%02ld C, Pres: %ld mbar (Raw: D1=%lu, D2=%lu)", 
             (long)(temp / 100), (long)((temp < 0 ? -temp : temp) % 100), (long)pressure_mbar,
             (unsigned long)raw_pressure, (unsigned long)raw_temperature);

    // --- Telemetry Recording Logic ---
    uint32_t now = hal_time_get_ms();

    // Write packed telemetry sample to FRAM
    telemetry_sample_t sample;
    sample.timestamp_ms = now;
    sample.temp_centi_c = (int16_t)temp;
    sample.pres_mbar = (uint16_t)pressure_mbar;

    uint32_t write_addr = TELEMETRY_FRAM_START_ADDR + sizeof(telemetry_block_header_t) + (g_telemetry_index * sizeof(telemetry_sample_t));
    
    // Write sample to FRAM
    hal_fram_write_it(write_addr, (const uint8_t *)&sample, sizeof(sample), NULL, NULL);
    
    g_telemetry_index++;

    // Update the sample_count in F-RAM block header
    hal_fram_write_it(TELEMETRY_FRAM_START_ADDR + offsetof(telemetry_block_header_t, sample_count),
                   (const uint8_t *)&g_telemetry_index, sizeof(g_telemetry_index), NULL, NULL);

    if (g_telemetry_index >= TELEMETRY_FRAM_MAX_SAMPLES) {
        LOG_INFO(OBC_COMPONENT_SENSORS, 
                 "F-RAM Telemetry Buffer Full. Flushing 124 samples (1KB) to Flash address 0x%08lX", 
                 (unsigned long)g_flash_write_addr);

        uint8_t temp_buf[1024];
        if (hal_fram_read(TELEMETRY_FRAM_START_ADDR, temp_buf, sizeof(temp_buf)) == HAL_FRAM_OK) {
            hal_flash_write(g_flash_write_addr, temp_buf, sizeof(temp_buf));
            g_flash_write_addr += sizeof(temp_buf);
        } else {
            LOG_ERROR(OBC_COMPONENT_SENSORS, "Failed to read telemetry buffer from F-RAM for flash flushing!");
        }
        
        g_sequence_number++;
        g_telemetry_index = 0; // wrap around

        // Write new header with updated flash address, sequence, and reset sample_count
        telemetry_block_header_t new_header = {
            .magic = 0x54454C4D,
            .sequence_number = g_sequence_number,
            .timestamp_ms = now,
            .flash_write_addr = g_flash_write_addr,
            .sample_count = 0
        };
        memset(new_header.reserved, 0, sizeof(new_header.reserved));

        hal_fram_write_it(TELEMETRY_FRAM_START_ADDR, (const uint8_t *)&new_header, sizeof(new_header), NULL, NULL);
    }
}

void sensors_flush_telemetry(void) {
    if (g_telemetry_index > 0) {
        LOG_INFO(OBC_COMPONENT_SENSORS, 
                 "Deep sleep flush. Flushing %lu samples to Flash address 0x%08lX", 
                 (unsigned long)g_telemetry_index, (unsigned long)g_flash_write_addr);

        uint8_t temp_buf[1024];
        if (hal_fram_read(TELEMETRY_FRAM_START_ADDR, temp_buf, sizeof(temp_buf)) == HAL_FRAM_OK) {
            hal_flash_write(g_flash_write_addr, temp_buf, sizeof(temp_buf));
            g_flash_write_addr += sizeof(temp_buf);
        } else {
            LOG_ERROR(OBC_COMPONENT_SENSORS, "Failed to read telemetry buffer from F-RAM for deep sleep flush!");
        }

        g_sequence_number++;
        g_telemetry_index = 0;

        telemetry_block_header_t new_header = {
            .magic = 0x54454C4D,
            .sequence_number = g_sequence_number,
            .timestamp_ms = hal_time_get_ms(),
            .flash_write_addr = g_flash_write_addr,
            .sample_count = 0
        };
        memset(new_header.reserved, 0, sizeof(new_header.reserved));

        hal_fram_write(TELEMETRY_FRAM_START_ADDR, (const uint8_t *)&new_header, sizeof(new_header));
    }
}
