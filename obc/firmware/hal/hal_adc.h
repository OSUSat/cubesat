#ifndef ADC_H
#define ADC_H

#include <stdint.h>

typedef enum {
    ADC_CHANNEL_0 = 0,
    ADC_CHANNEL_1,
    ADC_CHANNEL_2,
    ADC_CHANNEL_MAX
} adc_channel_t;

void hal_adc_init(void);
uint16_t hal_adc_read(adc_channel_t channel);

#endif // ADC_H
