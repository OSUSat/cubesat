#ifndef GPIO_MOCK_H
#define GPIO_MOCK_H

#include "gpio.h"
#include <stdint.h>

void mock_gpio_set_pin_state(uint8_t pin, gpio_state_t state);

#endif
