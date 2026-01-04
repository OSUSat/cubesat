/**
 * @file adc_mock.h
 * @brief Mock utilities for the ADC HAL.
 *
 * This module provides functions for injecting artificial ADC readings during
 * testing without actual hardware.
 */

#ifndef ADC_MOCK_H
#define ADC_MOCK_H

#include "hal_adc.h"

/**
 * @defgroup adc_mock ADC Mock
 * @ingroup adc
 * @brief Testing utilities for the ADC driver.
 * @{
 */

/**
 * @brief Set the mock ADC value for a given channel.
 *
 * @param[in] channel ADC channel to modify
 * @param[in] value Raw ADC value to return on reads
 */
void mock_adc_set_value(adc_channel_t channel, uint16_t value);

/** @} */ // end adc_mock

#endif // ADC_MOCK_H
