#ifndef POWER_PROFILES_H
#define POWER_PROFILES_H

#include "../config/eps_power_profiles.h"
#include <stddef.h>

typedef enum {
    POWER_PROFILE_NOMINAL,
    POWER_PROFILE_SAFE,
    // TODO: add more profiles as needed
} power_profile_t;

typedef struct {
    const power_rail_t *rails;
    int count;
} power_profile_info_t;

typedef enum {
    POWER_PROFILE_SUCCESS,
    POWER_PROFILE_ERROR_INVALID_PROFILE
} power_profile_status_t;

power_profile_status_t power_profiles_enable(power_profile_t profile);
power_profile_status_t power_profiles_disable(power_profile_t profile);

static power_profile_status_t _select_power_rails(power_profile_t profile,
                                                  power_profile_info_t *info);

#endif
