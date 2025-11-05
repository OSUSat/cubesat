#ifndef MPPT_CONTROLLER_H
#define MPPT_CONTROLLER_H

#include <stdbool.h>
#include <stdint.h>

#define NUM_MPPT_CHANNELS 6

typedef enum {
    MPPT_STATUS_OK,
    MPPT_STATUS_DISABLED,
    MPPT_STATUS_FAULT,
    MPPT_STATUS_UNDERVOLT,
    MPPT_STATUS_OVERTEMP
} mppt_status_t;

typedef struct {
    float input_voltage;
    float input_current;
    float output_voltage;
    float output_current;
    float power;
    mppt_status_t status;
    bool enabled;
    bool pgood;
} mppt_channel_t;

void mppt_init(void);
void mppt_enable(uint8_t ch);
void mppt_disable(uint8_t ch);
void mppt_update(void);
mppt_channel_t mppt_get_channel(uint8_t ch);
float mppt_get_total_power(void);

#endif
