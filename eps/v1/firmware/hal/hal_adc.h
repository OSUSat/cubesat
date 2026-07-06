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
 * @brief Available ADC input channels mapping to regular DMA ranks.
 */
typedef enum {
    ADC_CHANNEL_0 = 0,  /**< PC2 (ADC2_IN3) - Placeholder name */
    ADC_CHANNEL_1,      /**< PC3 (ADC2_IN4) - Placeholder name */
    ADC_CHANNEL_2,      /**< PA0 (ADC2_IN5) - Placeholder name */
    ADC_CHANNEL_3,      /**< PA1 (ADC2_IN6) - Placeholder name */
    ADC_CHANNEL_4,      /**< PA2 (ADC2_IN7) - Placeholder name */
    ADC_CHANNEL_5,      /**< PA3 (ADC2_IN8) - Placeholder name */
    ADC_CHANNEL_6,      /**< PA4 (ADC2_IN9) - Placeholder name */
    ADC_CHANNEL_7,      /**< PA5 (ADC2_IN10) - Placeholder name */
    ADC_CHANNEL_8,      /**< PA6 (ADC2_IN11) - Placeholder name */
    ADC_CHANNEL_9,      /**< PA7 (ADC2_IN12) - Placeholder name */
    ADC_CHANNEL_10,     /**< PB0 (ADC2_IN15) - Placeholder name */
    ADC_CHANNEL_MAX     /**< Total number of ADC channels */
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
