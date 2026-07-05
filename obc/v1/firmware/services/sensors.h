/**
 * @file sensors.h
 * @brief Sensor reading & logging service
 *
 * Adds support for several sensors including the MS5607-02BA
 */

#ifndef SENSORS_H
#define SENSORS_H

#include "hal_i2c.h"
#include "osusat/event_bus.h"
#include <stdint.h>

#define SENSORS_SERVICE_ID 0xD0D4

typedef struct {
    int64_t pressure;    /**< Pressure data read from barometer in mbar */
    int64_t temperature; /**< Temperature reading */
} barometer_t;

typedef struct {
    barometer_t barometer;
} sensor_data_t;

typedef enum {
    /**
     * @brief Published when valid sensor data is read from one or more sensors
     * Payload: sensor_data_t
     */
    SENSOR_DATA_RECEIVED = 0x10
} sensors_event_id_t;

#define SENSORS_EVENT_DATA_RECEIVED                                            \
    OSUSAT_BUILD_EVENT_ID(SENSORS_SERVICE_ID, SENSOR_DATA_RECEIVED)

/**
 * @brief Initialize the sensors service and MS5607-02BA sensor.
 */
void sensors_init(void);

/**
 * @brief Flush any pending telemetry samples to Flash before entering deep sleep.
 */
void sensors_flush_telemetry(void);

#endif // SENSORS_H
