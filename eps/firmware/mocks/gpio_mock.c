#include "gpio_mock.h"
#include "gpio.h"
#include <stdio.h>

#define MAX_MOCK_PINS 32

static gpio_state_t mock_pin_states[MAX_MOCK_PINS];
static gpio_mode_t mock_pin_modes[MAX_MOCK_PINS];

typedef struct {
    gpio_callback_t func;
    void *ctx;
} mock_callback_t;

static mock_callback_t mock_callbacks[MAX_MOCK_PINS];

/**
 * @brief Simulates the EXTI hardware line detector.
 * Checks if the state transition matches the configured Interrupt Mode.
 */
static void check_and_fire_interrupt(uint8_t pin, gpio_state_t old_state,
                                     gpio_state_t new_state) {
    if (mock_callbacks[pin].func == NULL) {
        return; // no one listening
    }

    bool fire = false;
    gpio_mode_t mode = mock_pin_modes[pin];

    // check rising edge (low -> high)
    if (old_state == GPIO_STATE_LOW && new_state == GPIO_STATE_HIGH) {
        if (mode == GPIO_MODE_IT_RISING ||
            mode == GPIO_MODE_IT_RISING_FALLING) {
            printf("MOCK IRQ: Rising Edge detected on Pin %d\n", pin);
            fire = true;
        }
    }

    // check falling edge (high -> low)
    else if (old_state == GPIO_STATE_HIGH && new_state == GPIO_STATE_LOW) {
        if (mode == GPIO_MODE_IT_FALLING ||
            mode == GPIO_MODE_IT_RISING_FALLING) {
            printf("MOCK IRQ: Falling Edge detected on Pin %d\n", pin);
            fire = true;
        }
    }

    if (fire) {
        // execute the registered callback (simulate ISR jumping to handler)
        mock_callbacks[pin].func(pin, mock_callbacks[pin].ctx);
    }
}

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

void gpio_register_callback(uint8_t pin, gpio_callback_t callback, void *ctx) {
    if (pin >= MAX_MOCK_PINS) {
        printf("MOCK ERROR: Pin %d out of bounds\n", pin);
        return;
    }

    printf("MOCK: Registering callback for Pin %d\n", pin);
    mock_callbacks[pin].func = callback;
    mock_callbacks[pin].ctx = ctx;
}

void gpio_write(uint8_t pin, gpio_state_t state) {
    if (pin >= MAX_MOCK_PINS) {
        printf("MOCK ERROR: Pin %d out of bounds\n", pin);
        return;
    }

    if (mock_pin_modes[pin] == GPIO_MODE_OUTPUT) {
        printf("MOCK: Writing %d to pin %d\n", state, pin);

        gpio_state_t old_state = mock_pin_states[pin];

        mock_pin_states[pin] = state;

        check_and_fire_interrupt(pin, old_state, state);
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

    return mock_pin_states[pin];
}

void gpio_toggle(uint8_t pin) {
    if (pin >= MAX_MOCK_PINS)
        return;

    if (mock_pin_modes[pin] == GPIO_MODE_OUTPUT) {
        gpio_state_t new_state = (mock_pin_states[pin] == GPIO_STATE_LOW)
                                     ? GPIO_STATE_HIGH
                                     : GPIO_STATE_LOW;

        gpio_write(pin, new_state);
    }
}

// mock-specific functions
void mock_gpio_set_pin_state(uint8_t pin, gpio_state_t state) {
    if (pin >= MAX_MOCK_PINS) {
        printf("MOCK ERROR: Pin %d out of bounds\n", pin);
        return;
    }

    printf("MOCK TEST: Forcing pin %d to state %d\n", pin, state);

    gpio_state_t old_state = mock_pin_states[pin];
    mock_pin_states[pin] = state;

    check_and_fire_interrupt(pin, old_state, state);
}
