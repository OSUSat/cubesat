#include "eps_config.h"
#include "hal_gpio.h"
#include "hal_time.h"
#include "iwdg.h"
#include "watchdog.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

void test_watchdog_init(void) {
    printf("Running test: %s\n", __func__);
    watchdog_t wd;
    wd.timeout_ms = 0;
    wd.last_pet_tick = 0;
    wd.enabled = false;

    hal_gpio_init();
    hal_gpio_set_mode(WATCHDOG_WDI_PIN, HAL_GPIO_MODE_OUTPUT);

    uint32_t current_tick = hal_time_get_ms();
    watchdog_init(&wd);

    assert(wd.timeout_ms == 512);
    assert(wd.enabled == true);
    assert(wd.last_pet_tick == current_tick);
    assert(hal_gpio_read(WATCHDOG_WDI_PIN) == HAL_GPIO_STATE_LOW);
    printf("Test passed.\n");
}

void test_watchdog_pet(void) {
    printf("Running test: %s\n", __func__);
    watchdog_t wd;

    hal_gpio_init();
    hal_gpio_set_mode(WATCHDOG_WDI_PIN, HAL_GPIO_MODE_OUTPUT);

    watchdog_init(&wd);

    uint32_t initial_refresh_count = mock_iwdg_refresh_count;
    uint32_t initial_tick = wd.last_pet_tick;

    // simulate time passing by sleeping for 2 milliseconds
    usleep(2000);

    assert(hal_gpio_read(WATCHDOG_WDI_PIN) == HAL_GPIO_STATE_LOW);
    watchdog_pet(&wd);

    assert(mock_iwdg_refresh_count == initial_refresh_count + 1);
    assert(wd.last_pet_tick >= initial_tick + 2);
    assert(hal_gpio_read(WATCHDOG_WDI_PIN) == HAL_GPIO_STATE_HIGH);

    // pet again to verify it toggles back to low
    watchdog_pet(&wd);
    assert(hal_gpio_read(WATCHDOG_WDI_PIN) == HAL_GPIO_STATE_LOW);

    printf("Test passed.\n");
}

int main(void) {
    test_watchdog_init();
    test_watchdog_pet();

    return 0;
}
