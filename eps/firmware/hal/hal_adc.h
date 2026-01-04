/**
 * @file hal_adc.h
 * @brief ADC hardware abstraction public API.
 *
 * This driver provides an interface for reading analog values from the ADC
 * peripherals. It returns raw conversion values, typically 10–12 bits depending
 * on the hardware.
 */

#ifndef ADC_H
#define ADC_H

#include <stdint.h>

/**
 * @defgroup adc ADC
 * @brief Handles analog-to-digital conversions.
 * @{
 */

/**
 * @defgroup adc_types Types
 * @ingroup adc @brief Types used by the ADC driver.
 *
 * @{
 */

/**
 * @enum adc_channel_t
 * @brief Available ADC input channels.
 */
typedef enum {
    ADC_CHANNEL_0 = 0, /**< ADC channel 0 */
    ADC_CHANNEL_1,     /**< ADC channel 1 */
    ADC_CHANNEL_2,     /**< ADC channel 2 */
    ADC_CHANNEL_3,     /**< ADC channel 3 */
    ADC_CHANNEL_MAX    /**< Total number of ADC channels */
} adc_channel_t;

/** @} */ // end adc_types

/**
 * @defgroup adc_api Public API
 * @ingroup adc
 * @brief External interface for interacting with the ADC driver.
 *
 * @{
 */

/**
 * @brief Initialize the ADC subsystem.
 *
 * This must be called before sampling any channels.
 */
void hal_adc_init(void);

/**
 * @brief Read a raw ADC value from the specified channel.
 *
 * @param[in] channel The ADC channel to read
 *
 * @return Raw ADC conversion value (resolution hardware-dependent)
 */
uint16_t hal_adc_read(adc_channel_t channel);

/** @} */ // end adc_api

/** @} */ // end adc

#endif // ADC_H
