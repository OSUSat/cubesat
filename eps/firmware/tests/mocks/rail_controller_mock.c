#include "rail_controller_mock.h"
#include "rail_controller.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define MAX_RAILS 16
static power_rail_t enabled_rails[MAX_RAILS];
static int enabled_count = 0;

static power_rail_t disabled_rails[MAX_RAILS];
static int disabled_count = 0;

void rail_controller_enable(rail_controller_t *controller, power_rail_t rail) {
    (void)controller;

    if (enabled_count < MAX_RAILS) {
        enabled_rails[enabled_count++] = rail;
    }
}

void rail_controller_disable(rail_controller_t *controller, power_rail_t rail) {
    (void)controller;

    if (disabled_count < MAX_RAILS) {
        disabled_rails[disabled_count++] = rail;
    }
}

int mock_get_enabled_count() { return enabled_count; }

power_rail_t mock_get_enabled_rail(int index) { return enabled_rails[index]; }

int mock_get_disabled_count() { return disabled_count; }

power_rail_t mock_get_disabled_rail(int index) { return disabled_rails[index]; }

void mock_reset_rail_controller() {
    enabled_count = 0;
    disabled_count = 0;
    memset(enabled_rails, 0, sizeof(enabled_rails));
    memset(disabled_rails, 0, sizeof(disabled_rails));
}

void rail_controller_init(rail_controller_t *controller) {
    (void)controller;
    printf("MOCK: rail_controller_init\n");
}

void rail_controller_update(rail_controller_t *controller) { (void)controller; }

rail_t rail_controller_get_rail(rail_controller_t *controller,
                                power_rail_t rail) {
    (void)controller;
    (void)rail;

    return (rail_t){0};
}
