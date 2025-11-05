#ifndef RAIL_CONTROLLER_H
#define RAIL_CONTROLLER_H

#include "../config/eps_config.h"
#include <stdbool.h>
#include <stdint.h>

typedef enum {
    RAIL_STATUS_OK,
    RAIL_STATUS_DISABLED,
    RAIL_STATUS_OVERCURRENT,
    RAIL_STATUS_UNDERVOLTAGE,
    RAIL_STATUS_FAULT
} rail_status_t;

typedef struct {
    float voltage;
    float current;
    rail_status_t status;
    bool enabled;
} rail_t;

void rail_controller_init(void);
void rail_controller_enable(power_rail_t rail);
void rail_controller_disable(power_rail_t rail);
void rail_controller_update(void);
rail_t rail_controller_get_rail(power_rail_t rail);

#endif
