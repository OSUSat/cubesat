#ifndef EPS_POWER_PROFILES_H
#define EPS_POWER_PROFILES_H

#include "eps_config.h"

const power_rail_t nominal_mode_rails[] = {RAIL_OBC, RAIL_RADIO, RAIL_GPS,
                                           RAIL_PAYLOAD_1};

const power_rail_t safe_mode_rails[] = {RAIL_OBC};

// ... other power profiles ...

#endif
