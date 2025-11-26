#ifndef GPIO_H
#define GPIO_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    GPIO_MODE_INPUT,
    GPIO_MODE_OUTPUT,
} gpio_mode_t;

typedef enum {
    GPIO_STATE_LOW,
    GPIO_STATE_HIGH,
} gpio_state_t;

void gpio_init(void);
void gpio_set_mode(uint8_t pin, gpio_mode_t mode);
void gpio_write(uint8_t pin, gpio_state_t state);
gpio_state_t gpio_read(uint8_t pin);
void gpio_toggle(uint8_t pin);

#endif
