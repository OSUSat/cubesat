/**
 * @file hal_adc.c
 * @brief HAL driver for ADC
 */

#include "hal_adc.h"
#include "stm32l4xx_hal.h"

extern ADC_HandleTypeDef hadc2;

#define ADC_BUFFER_SIZE 11
static uint16_t adc_buffer[ADC_BUFFER_SIZE];
static uint8_t adc_initialized = 0;

void hal_adc_init(void) {
    if (adc_initialized) {
        return;
    }
    // Start circular DMA regular conversions for ADC2
    HAL_ADC_Start_DMA(&hadc2, (uint32_t*)adc_buffer, ADC_BUFFER_SIZE);
    adc_initialized = 1;
}

uint16_t hal_adc_read(adc_channel_t channel) {
    if (!adc_initialized) {
        hal_adc_init();
    }
    if (channel >= ADC_CHANNEL_MAX) {
        return 0;
    }
    return adc_buffer[channel];
}
