#include "hal_gpio_mock.h"
#include "hal_gpio_types.h"
#include <stdio.h>
#include <string.h>

#define MAX_MOCK_PINS 32

static gpio_pin_t mock_pins[MAX_MOCK_PINS];
static bool mock_initialized = false;

/**
 * @brief Simulates the EXTI hardware line detector.
 * Checks if the state transition matches the configured Interrupt Mode.
 */
static void check_and_fire_interrupt(uint8_t pin, gpio_state_t old_state,
                                     gpio_state_t new_state) {
    if (pin >= MAX_MOCK_PINS) {
        return;
    }

    if (!mock_pins[pin].irq_enabled || mock_pins[pin].cb == NULL) {
        return; // no callback registered or IRQ not enabled
    }

    bool fire = false;
    gpio_mode_t mode = mock_pins[pin].mode;

    // check rising edge (low -> high)
    if (old_state == HAL_GPIO_STATE_LOW && new_state == HAL_GPIO_STATE_HIGH) {
        if (mode == HAL_GPIO_MODE_IT_RISING ||
            mode == HAL_GPIO_MODE_IT_RISING_FALLING) {
            printf("MOCK IRQ: Rising Edge detected on Pin %d\n", pin);
            fire = true;
        }
    }
    // check falling edge (high -> low)
    else if (old_state == HAL_GPIO_STATE_HIGH &&
             new_state == HAL_GPIO_STATE_LOW) {
        if (mode == HAL_GPIO_MODE_IT_FALLING ||
            mode == HAL_GPIO_MODE_IT_RISING_FALLING) {
            printf("MOCK IRQ: Falling Edge detected on Pin %d\n", pin);
            fire = true;
        }
    }

    if (fire) {
        // execute the registered callback (simulate ISR jumping to handler)
        mock_pins[pin].cb(pin, mock_pins[pin].ctx);
    }
}

void hal_gpio_init(void) {
    printf("MOCK: GPIO initialized\n");

    memset(mock_pins, 0, sizeof(mock_pins));

    for (int i = 0; i < MAX_MOCK_PINS; i++) {
        mock_pins[i].mode = HAL_GPIO_MODE_INPUT;
        mock_pins[i].pull = HAL_GPIO_NO_PULL;
        mock_pins[i].irq_enabled = false;
        mock_pins[i].cb = NULL;
        mock_pins[i].ctx = NULL;
    }

    mock_initialized = true;
}

void hal_gpio_set_mode(uint8_t pin, gpio_mode_t mode) {
    if (pin >= MAX_MOCK_PINS || !mock_initialized) {
        printf("MOCK ERROR: Pin %d out of bounds or not initialized\n", pin);
        return;
    }

    printf("MOCK: Setting pin %d to mode %d\n", pin, mode);

    if (mock_pins[pin].irq_enabled) {
        mock_pins[pin].irq_enabled = false;
        printf("MOCK: Releasing IRQ for pin %d\n", pin);
    }

    mock_pins[pin].mode = mode;

    // enable IRQ for interrupt modes
    if (mode == HAL_GPIO_MODE_IT_RISING || mode == HAL_GPIO_MODE_IT_FALLING ||
        mode == HAL_GPIO_MODE_IT_RISING_FALLING) {
        mock_pins[pin].irq_enabled = true;
        printf("MOCK: IRQ enabled for pin %d\n", pin);
    }
}

void hal_gpio_register_callback(uint8_t pin, gpio_callback_t callback,
                                void *ctx) {
    if (pin >= MAX_MOCK_PINS || !mock_initialized) {
        printf("MOCK ERROR: Pin %d out of bounds or not initialized\n", pin);
        return;
    }

    printf("MOCK: Registering callback for Pin %d\n", pin);
    mock_pins[pin].cb = callback;
    mock_pins[pin].ctx = ctx;
}

void hal_gpio_write(uint8_t pin, gpio_state_t state) {
    if (pin >= MAX_MOCK_PINS || !mock_initialized) {
        printf("MOCK ERROR: Pin %d out of bounds or not initialized\n", pin);
        return;
    }

    if (mock_pins[pin].mode == HAL_GPIO_MODE_OUTPUT) {
        printf("MOCK: Writing %d to pin %d\n", state, pin);

        gpio_state_t old_state = mock_pins[pin].state;
        mock_pins[pin].state = state;

        check_and_fire_interrupt(pin, old_state, state);
    } else {
        printf("MOCK WARN: Attempted to write to pin %d which is not in OUTPUT "
               "mode\n",
               pin);
    }
}

gpio_state_t hal_gpio_read(uint8_t pin) {
    if (pin >= MAX_MOCK_PINS || !mock_initialized) {
        printf("MOCK ERROR: Pin %d out of bounds or not initialized\n", pin);
        return HAL_GPIO_STATE_LOW;
    }

    return mock_pins[pin].state;
}

void hal_gpio_toggle(uint8_t pin) {
    if (pin >= MAX_MOCK_PINS || !mock_initialized) {
        return;
    }

    if (mock_pins[pin].mode == HAL_GPIO_MODE_OUTPUT) {
        gpio_state_t new_state = (mock_pins[pin].state == HAL_GPIO_STATE_LOW)
                                     ? HAL_GPIO_STATE_HIGH
                                     : HAL_GPIO_STATE_LOW;
        hal_gpio_write(pin, new_state);
    }
}

// mock-specific functions
void mock_gpio_set_pin_state(uint8_t pin, gpio_state_t state) {
    if (pin >= MAX_MOCK_PINS) {
        printf("MOCK ERROR: Pin %d out of bounds\n", pin);
        return;
    }

    printf("MOCK TEST: Forcing pin %d to state %d\n", pin, state);
    gpio_state_t old_state = mock_pins[pin].state;
    mock_pins[pin].state = state;
    check_and_fire_interrupt(pin, old_state, state);
}

gpio_pin_t *mock_gpio_get_pin(uint8_t pin) {
    if (pin >= MAX_MOCK_PINS) {
        return NULL;
    }
    return &mock_pins[pin];
}
