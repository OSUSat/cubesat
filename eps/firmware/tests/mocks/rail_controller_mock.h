#ifndef RAIL_CONTROLLER_MOCK_H
#define RAIL_CONTROLLER_MOCK_H

#include "rail_controller.h"

int mock_get_enabled_count();
power_rail_t mock_get_enabled_rail(int index);
int mock_get_disabled_count();
power_rail_t mock_get_disabled_rail(int index);
void mock_reset_rail_controller();

#endif
