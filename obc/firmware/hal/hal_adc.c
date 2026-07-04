/**
 * @file hal_adc.c
 * @brief HAL driver skeleton for ADC on OBC.
 */

#include "hal_adc.h"
#include <stdint.h>

static uint8_t adc_initialized = 0;

void hal_adc_init(void) { adc_initialized = 1; }

uint16_t hal_adc_read(adc_channel_t channel) {
    if (!adc_initialized) {
        hal_adc_init();
    }

    if (channel >= ADC_CHANNEL_MAX) {
        return 0;
    }

    // Return a dummy value/placeholder since ADC is not configured yet

    return 2048;
}
