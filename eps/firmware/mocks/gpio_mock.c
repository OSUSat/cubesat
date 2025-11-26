#include "gpio_mock.h"
#include "gpio.h"
#include <stdio.h>

#define MAX_MOCK_PINS 32

static gpio_state_t mock_pin_states[MAX_MOCK_PINS];
static gpio_mode_t mock_pin_modes[MAX_MOCK_PINS];

void gpio_init(void) {
    printf("MOCK: GPIO initialized\n");

    for (int i = 0; i < MAX_MOCK_PINS; i++) {
        mock_pin_states[i] = GPIO_STATE_LOW;
        mock_pin_modes[i] = GPIO_MODE_INPUT;
    }
}

void gpio_set_mode(uint8_t pin, gpio_mode_t mode) {
    if (pin >= MAX_MOCK_PINS) {
        printf("MOCK ERROR: Pin %d out of bounds\n", pin);
        return;
    }

    printf("MOCK: Setting pin %d to mode %d\n", pin, mode);
    mock_pin_modes[pin] = mode;
}

void gpio_write(uint8_t pin, gpio_state_t state) {
    if (pin >= MAX_MOCK_PINS) {
        printf("MOCK ERROR: Pin %d out of bounds\n", pin);
        return;
    }

    if (mock_pin_modes[pin] == GPIO_MODE_OUTPUT) {
        printf("MOCK: Writing %d to pin %d\n", state, pin);
        mock_pin_states[pin] = state;
    } else {
        printf("MOCK WARN: Attempted to write to pin %d which is not in OUTPUT "
               "mode\n",
               pin);
    }
}

gpio_state_t gpio_read(uint8_t pin) {
    if (pin >= MAX_MOCK_PINS) {
        printf("MOCK ERROR: Pin %d out of bounds\n", pin);
        return GPIO_STATE_LOW;
    }

    printf("MOCK: Reading from pin %d, value is %d\n", pin,
           mock_pin_states[pin]);

    return mock_pin_states[pin];
}

void gpio_toggle(uint8_t pin) {
    if (pin >= MAX_MOCK_PINS) {
        printf("MOCK ERROR: Pin %d out of bounds\n", pin);
        return;
    }

    if (mock_pin_modes[pin] == GPIO_MODE_OUTPUT) {
        printf("MOCK: Toggling pin %d\n", pin);

        mock_pin_states[pin] = (mock_pin_states[pin] == GPIO_STATE_LOW)
                                   ? GPIO_STATE_HIGH
                                   : GPIO_STATE_LOW;
    } else {
        printf("MOCK WARN: Attempted to toggle pin %d which is not in OUTPUT "
               "mode\n",
               pin);
    }
}

// mock-specific functions
void mock_gpio_set_pin_state(uint8_t pin, gpio_state_t state) {
    if (pin >= MAX_MOCK_PINS) {
        printf("MOCK ERROR: Pin %d out of bounds\n", pin);
        return;
    }

    printf("MOCK: Forcing pin %d to state %d for testing\n", pin, state);

    mock_pin_states[pin] = state;
}
