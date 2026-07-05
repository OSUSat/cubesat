# Refactoring Polling-Based Modules to be Event Driven

## Overview

These guidelines outline a general decision framework for converting polling-based firmware modules into event-driven modules using the OSUSat core libraries.

## Auditing Non-Refactored Modules

To begin refactoring a polling module, we first need to audit it and classify its interface into several categories.

1. Read over the current interface & its corresponding documentation
2. Classify every function or type into 3 categories:

| Type | Example | Action |
|---|---|---|
| Question | `is_data_ready()`, `get_status()`, `check_error()` | Delete and replace with an event pushed by the service |
| Command | `start_process()`, `set_config()`, `reset()` | Keep these, since they are a specific instruction. However, create a new event subscription in the service that listens for requested changes from the application. The application won't directly command services, and instead requests changes over the event bus. Then individual services will call their commands to move towards the requested state. |
| Loop | `update()`, `process_logic()` | Hide these in the implementation. Make a similar or equivalent function static in the source file and call it from a new system tick handler. |

## Refactoring Headers

This is a general guideline for refactoring headers:

```c
/**
 * @file [service_name].h
 */
#ifndef [SERVICE]_H
#define [SERVICE]_H

#include "osusat_event_bus.h"

// 1. PICK A UNIQUE ID (16-bit Hex)
// Examples: 0xBA77 (Batt), 0x5014 (Solar), or just random
#define [SERVICE]_UID 0x____

// 2. DEFINE LOCAL EVENTS (Enum)
typedef enum {
    [SERVICE]_STATUS_CHANGE = 0x10,
    [SERVICE]_ERROR,
    [SERVICE]_TASK_COMPLETE
} [service]_code_t;

// 3. BUILD PUBLIC COMPOSITE IDs
#define [SERVICE]_EVENT_STATUS_CHANGE OSUSAT_BUILD_EVENT_ID([SERVICE]_UID, [SERVICE]_STATUS_CHANGE)
#define [SERVICE]_EVENT_ERROR OSUSAT_BUILD_EVENT_ID([SERVICE]_UID, [SERVICE]_ERROR)
#define [SERVICE]_EVENT_TASK_COMPLETE OSUSAT_BUILD_EVENT_ID([SERVICE]_UID, [SERVICE]_TASK_COMPLETE)

// 4. DEFINE STATE STRUCT
typedef struct {
    // Public read-only state (snapshot)
    float data;
    bool is_busy;
} [service]_status_t;

typedef struct {
    [service]_status_t status;
    bool initialized;
} [service]_t;

// 5. DEFINE API (here, just keep the init and commands)
void [service]_init([service]_t *self);
void [service]_start_action([service]_t *self);
void [service]_stop_action([service]_t *self);

#endif
```

## Implementation

This is a simple template for the implementation of a service, updated for an event driven architecture:

```c
/**
 * @file [service_name].c
 */
#include "[service_name].h"
#include <string.h>

// 1. CONFIG: Frequency Prescaler
// How many System Ticks to skip?
// Example: If SysTick is 100Hz and we want 10Hz update -> 10.
#define UPDATE_PRESCALER 10

// 2. PRIVATE PROTOTYPES
static void handle_tick(const osusat_event_t *e, void *ctx);
static void perform_update([service]_t *self);

// 3. PUBLIC API
void [service]_init([service]_t *self) {
    if (!self) return;

    // clear state
    memset(self, 0, sizeof([service]_t));

    // hardware actions (HAL)
    // hal_gpio_init(...);

    self->initialized = true;

    // subscribe to the heartbeat.
    // pass 'self' as context so we know which struct to update.
    osusat_event_bus_subscribe(EVENT_SYSTICK, handle_tick, self);
}

void [service]_start_action([service]_t *self) {
    if (!self || !self->initialized) return;

    // 1. perform hardware action
    // hal_gpio_write(PIN_START, 1);

    // 2. update internal state
    self->status.is_busy = true;

    // 3. notify system
    osusat_event_bus_publish([SERVICE]_EVENT_STATUS_CHANGE,
                              &self->status,
                              sizeof([service]_status_t));
}

// 4. PRIVATE LOGIC
static void handle_tick(const osusat_event_t *e, void *ctx) {
    (void)e;
    [service]_t *self = ([service]_t *)ctx;
    if (!self || !self->initialized) return;

    // Prescaler Logic
    static uint32_t tick_count = 0; // Initialize to offset if needed
    tick_count++;

    if (tick_count >= UPDATE_PRESCALER) {
        tick_count = 0;
        perform_update(self);
    }
}

static void perform_update([service]_t *self) {
    // 1. READ SENSORS
    // float val = hal_adc_read(...);

    // 2. UPDATE STATE
    // self->status.data = val;

    // 3. CHECK THRESHOLDS & PUBLISH
    // if (val > LIMIT) {
    //     osusat_event_bus_publish([SERVICE]_EVENT_ERROR, &val, sizeof(float));
    // }
}
```

exit
