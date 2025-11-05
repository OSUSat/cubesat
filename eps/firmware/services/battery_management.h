#ifndef BATTERY_MANAGEMENT_H
#define BATTERY_MANAGEMENT_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    float voltage;     // pack voltage
    float current;     // charge (+) or discharge (-)
    float temperature; // average pack temperature
    float soc;         // state of Charge (%)
    float soh;         // state of Health (%)
    bool charging;     // charging active?
    bool balancing;    // balancing circuits enabled?
} battery_status_t;

// public API
void battery_init(void);
void battery_update(void);
bool battery_self_check_ok(void);
bool battery_needs_charge(void);
bool battery_full(void);
void battery_charge_control(void);
void battery_protect_mode(void);
battery_status_t battery_get_status(void);
float battery_get_voltage(void);
float battery_get_current(void);

#endif
