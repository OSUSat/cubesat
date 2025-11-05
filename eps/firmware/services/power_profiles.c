#include "power_profiles.h"
#include "rail_controller.h"

power_profile_status_t power_profiles_enable(power_profile_t profile) {
    power_profile_info_t info;
    power_profile_status_t status = _select_power_rails(profile, &info);

    if (status != POWER_PROFILE_SUCCESS) {
        return status;
    }

    for (int i = 0; i < info.count; i++) {
        rail_controller_enable(info.rails[i]);
    }

    return POWER_PROFILE_SUCCESS;
}

power_profile_status_t power_profiles_disable(power_profile_t profile) {
    power_profile_info_t info;
    power_profile_status_t status = _select_power_rails(profile, &info);

    if (status != POWER_PROFILE_SUCCESS) {
        return status;
    }

    for (int i = 0; i < info.count; i++) {
        rail_controller_disable(info.rails[i]);
    }

    return POWER_PROFILE_SUCCESS;
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
