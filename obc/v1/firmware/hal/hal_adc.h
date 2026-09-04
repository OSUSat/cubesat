/**
 * @file hal_adc.h
 * @brief Analog-to-Digital Converter (ADC) Hardware Abstraction Layer for OBC.
 *
 * Provides functions to initialize and read analog input channels.
 */

#ifndef ADC_H
#define ADC_H

#include <stdint.h>

/**
 * @defgroup obc_adc ADC Driver
 * @brief Analog-to-Digital Converter HAL interface.
 * @{
 */

/**
 * @enum adc_channel_t
 * @brief ADC channel identifiers.
 */
typedef enum {
    ADC_CHANNEL_0 = 0, /**< Analog Channel 0 */
    ADC_CHANNEL_1,     /**< Analog Channel 1 */
    ADC_CHANNEL_2,     /**< Analog Channel 2 */
    ADC_CHANNEL_MAX    /**< Maximum channel count */
} adc_channel_t;

/**
 * @brief Initialize the ADC peripheral.
 */
void hal_adc_init(void);

/**
 * @brief Read raw conversion value from the specified ADC channel.
 *
 * @param[in] channel Channel index to sample.
 * @return 12-bit raw conversion sample value.
 */
uint16_t hal_adc_read(adc_channel_t channel);

/** @} */

#endif // ADC_H
