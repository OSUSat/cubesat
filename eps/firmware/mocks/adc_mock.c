/**
 * @file adc_mock.c
 * @brief Mock implementation for the ADC HAL.
 *
 * This mock simulates ADC readings for unit tests and firmware
 * development without actual hardware.
 */

#include "adc_mock.h"
#include "adc.h"
#include <stdint.h>
#include <stdio.h>

#define MAX_ADC_CHANNELS ADC_CHANNEL_MAX

static uint16_t mock_adc_values[MAX_ADC_CHANNELS];

void adc_init(void) {
    printf("MOCK: ADC initialized\n");

    for (int i = 0; i < MAX_ADC_CHANNELS; i++) {
        mock_adc_values[i] = 0;
    }
}

uint16_t adc_read(adc_channel_t channel) {
    if (channel >= MAX_ADC_CHANNELS) {
        printf("MOCK ERROR: ADC channel %d out of bounds\n", channel);
        return 0;
    }

    uint16_t value = mock_adc_values[channel];
    printf("MOCK: Reading ADC channel %d => %u\n", channel, value);
    return value;
}

void mock_adc_set_value(adc_channel_t channel, uint16_t value) {
    if (channel >= MAX_ADC_CHANNELS) {
        printf("MOCK ERROR: ADC channel %d out of bounds\n", channel);
        return;
    }

    printf("MOCK: Setting ADC channel %d to %u\n", channel, value);
    mock_adc_values[channel] = value;
}
