#include "power_profiles.h"
#include "eps_power_profiles.h"
#include "events.h"
#include "osusat/event_bus.h"
#include "rail_controller.h"
#include <string.h>

static power_profile_status_t _select_power_rails(power_profile_t profile,
                                                  power_profile_info_t *info);
static void handle_profile_request_event(const osusat_event_t *e, void *ctx);

void power_profiles_init(power_profiles_t *profiles,
                         rail_controller_t *controller) {
    if (profiles == NULL || controller == NULL) {
        return;
    }

    memset(profiles, 0, sizeof(power_profiles_t));

    profiles->rail_controller = controller;
    profiles->initialized = true;

    // set initial state to SAFE, gradual bringup
    profiles->current_profile = POWER_PROFILE_SAFE;

    power_profiles_enable(profiles, profiles->current_profile);

    osusat_event_bus_subscribe(APP_EVENT_REQUEST_POWER_PROFILE_NOMINAL,
                               handle_profile_request_event, profiles);
    osusat_event_bus_subscribe(APP_EVENT_REQUEST_POWER_PROFILE_SAFE,
                               handle_profile_request_event, profiles);
}

power_profile_status_t power_profiles_enable(power_profiles_t *profiles,
                                             power_profile_t profile) {
    power_profile_info_t info;
    power_profile_status_t status = _select_power_rails(profile, &info);

    if (status != POWER_PROFILE_SUCCESS) {
        return status;
    }

    for (int i = 0; i < info.count; i++) {
        rail_controller_enable(profiles->rail_controller, info.rails[i]);
    }

    return POWER_PROFILE_SUCCESS;
}

power_profile_status_t power_profiles_disable(power_profiles_t *profiles,
                                              power_profile_t profile) {
    power_profile_info_t info;
    power_profile_status_t status = _select_power_rails(profile, &info);

    if (status != POWER_PROFILE_SUCCESS) {
        return status;
    }

    for (int i = 0; i < info.count; i++) {
        rail_controller_disable(profiles->rail_controller, info.rails[i]);
    }

    return POWER_PROFILE_SUCCESS;
}

static void handle_profile_request_event(const osusat_event_t *e, void *ctx) {
    power_profiles_t *profiles = (power_profiles_t *)ctx;
    power_profile_t requested_profile;

    switch (e->id) {
    case APP_EVENT_REQUEST_POWER_PROFILE_NOMINAL:
        requested_profile = POWER_PROFILE_NOMINAL;
        break;

    case APP_EVENT_REQUEST_POWER_PROFILE_SAFE:
        requested_profile = POWER_PROFILE_SAFE;
        break;

    default:
        return; // unknown event
    }

    if (profiles->current_profile != requested_profile) {
        power_profiles_disable(profiles, profiles->current_profile);
        profiles->current_profile = requested_profile;
        power_profiles_enable(profiles, profiles->current_profile);
    }
}

static power_profile_status_t _select_power_rails(power_profile_t profile,
                                                  power_profile_info_t *info) {
    switch (profile) {
    case POWER_PROFILE_NOMINAL:
        *info = (power_profile_info_t){.rails = nominal_mode_rails,
                                       .count = sizeof(nominal_mode_rails) /
                                                sizeof(nominal_mode_rails[0])};

        return POWER_PROFILE_SUCCESS;

    case POWER_PROFILE_SAFE:
        *info = (power_profile_info_t){.rails = safe_mode_rails,
                                       .count = sizeof(safe_mode_rails) /
                                                sizeof(safe_mode_rails[0])};

        return POWER_PROFILE_SUCCESS;

    default:
        *info = (power_profile_info_t){.rails = NULL, .count = 0};

        return POWER_PROFILE_ERROR_INVALID_PROFILE;
    }
}
