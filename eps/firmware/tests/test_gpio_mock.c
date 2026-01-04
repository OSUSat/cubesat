#include "hal_gpio.h"
#include "hal_gpio_mock.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

static bool callback_fired = false;
static uint8_t callback_pin = 0;

void test_callback(uint8_t pin, void *ctx) {
    (void)ctx;
    callback_fired = true;
    callback_pin = pin;
}

void reset_callback_state(void) {
    callback_fired = false;
    callback_pin = 0;
}

void test_gpio_write_read(void) {
    printf("Running test: %s\n", __func__);
    hal_gpio_init();

    hal_gpio_set_mode(5, GPIO_MODE_OUTPUT);
    hal_gpio_write(5, GPIO_STATE_HIGH);
    assert(hal_gpio_read(5) == GPIO_STATE_HIGH);

    hal_gpio_write(5, GPIO_STATE_LOW);
    assert(hal_gpio_read(5) == GPIO_STATE_LOW);
    printf("Test passed.\n");
}

void test_gpio_toggle(void) {
    printf("Running test: %s\n", __func__);
    hal_gpio_init();

    hal_gpio_set_mode(3, GPIO_MODE_OUTPUT);
    hal_gpio_write(3, GPIO_STATE_LOW);
    assert(hal_gpio_read(3) == GPIO_STATE_LOW);

    hal_gpio_toggle(3);
    assert(hal_gpio_read(3) == GPIO_STATE_HIGH);

    hal_gpio_toggle(3);
    assert(hal_gpio_read(3) == GPIO_STATE_LOW);
    printf("Test passed.\n");
}

void test_gpio_interrupts(void) {
    printf("Running test: %s\n", __func__);
    hal_gpio_init();
    reset_callback_state();

    // test rising edge interrupt
    hal_gpio_set_mode(8, GPIO_MODE_IT_RISING);
    hal_gpio_register_callback(8, test_callback, NULL);

    // force a low-to-high transition
    mock_gpio_set_pin_state(8, GPIO_STATE_LOW);
    mock_gpio_set_pin_state(8, GPIO_STATE_HIGH);

    assert(callback_fired);
    assert(callback_pin == 8);

    reset_callback_state();

    // test falling edge interrupt
    hal_gpio_set_mode(9, GPIO_MODE_IT_FALLING);
    hal_gpio_register_callback(9, test_callback, NULL);
    mock_gpio_set_pin_state(9, GPIO_STATE_HIGH);
    mock_gpio_set_pin_state(9, GPIO_STATE_LOW); // falling edge

    assert(callback_fired);
    assert(callback_pin == 9);

    reset_callback_state();

    // test no interrupt when mode doesn't match
    hal_gpio_set_mode(10, GPIO_MODE_IT_RISING);
    hal_gpio_register_callback(10, test_callback, NULL);
    mock_gpio_set_pin_state(10, GPIO_STATE_HIGH);
    reset_callback_state();
    mock_gpio_set_pin_state(10, GPIO_STATE_LOW); // falling edge

    assert(!callback_fired);

    printf("Test passed.\n");
}

int main(void) {
    test_gpio_write_read();
    test_gpio_toggle();
    test_gpio_interrupts();

    return 0;
}
