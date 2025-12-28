#include "app/command_handler.h"
#include "mppt_controller.h"
#include "osusat/event_bus.h"
#include "power_policies.h"
#include "rail_controller.h"
#include "services/power_profiles.h"

// event bus
#define EVENT_QUEUE_SIZE 16
static osusat_event_t event_queue[EVENT_QUEUE_SIZE];

// applications
static command_handler_t command_handler;
static power_policies_t power_policies;

// services
static rail_controller_t rail_controller;
static power_profiles_t power_profiles_service;
static mppt_t mppt_controller_service;

int main() {
    osusat_event_bus_init(event_queue, EVENT_QUEUE_SIZE);

    // initialize services
    rail_controller_init(&rail_controller);
    power_profiles_init(&power_profiles_service, &rail_controller);
    mppt_init(&mppt_controller_service);

    command_handler_init(&command_handler);
    power_policies_init(&power_policies);

    while (1) {
        osusat_event_bus_process();
    }

    return 0;
}
