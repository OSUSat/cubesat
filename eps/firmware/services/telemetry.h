#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <stdint.h>

typedef struct {
    // TODO: this will be a larger struct containing all service telemetry data
} eps_telemetry_t;

void telemetry_init(void);
void telemetry_update(void);
eps_telemetry_t telemetry_get_all(void);

#endif
