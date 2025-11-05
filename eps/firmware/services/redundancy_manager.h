#ifndef REDUNDANCY_MANAGER_H
#define REDUNDANCY_MANAGER_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    FAULT_SOURCE_BATTERY,
    FAULT_SOURCE_MPPT,
    FAULT_SOURCE_RAIL,
    FAULT_SOURCE_SENSOR,
    // TODO: implement more as needed
} fault_source_t;

// generic fault code
// each source can define its own specific codes
typedef uint32_t fault_code_t;

typedef enum {
    SYSTEM_HEALTH_OK,
    SYSTEM_HEALTH_DEGRADED,
    SYSTEM_HEALTH_FAULT
} system_health_t;

void redundancy_manager_init(void);
void redundancy_manager_report_fault(fault_source_t source, fault_code_t code);
void redundancy_manager_clear_fault(fault_source_t source, fault_code_t code);
system_health_t redundancy_manager_get_system_health(void);
void redundancy_manager_update(void);

#endif
