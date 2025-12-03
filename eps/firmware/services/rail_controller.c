#include "rail_controller.h"
#include <stdio.h>

void rail_controller_init(rail_controller_t *controller) {
    (void)controller;
    printf("STUB: %s\n", __func__);
}

void rail_controller_update(rail_controller_t *controller) {
    (void)controller;
}

rail_t rail_controller_get_rail(rail_controller_t *controller, power_rail_t rail) {
    (void)controller;
    (void)rail;
    return (rail_t){0};
}

void rail_controller_enable(rail_controller_t *controller, power_rail_t rail) {
    (void)controller;
    (void)rail;
    printf("STUB: rail_controller_enable for rail %d\n", rail);
}

void rail_controller_disable(rail_controller_t *controller, power_rail_t rail) {
    (void)controller;
    (void)rail;
    printf("STUB: rail_controller_disable for rail %d\n", rail);
}
