#ifndef EPS_CONFIG_H
#define EPS_CONFIG_H

typedef enum { // these rails correspond to the hardware rails
    RAIL_OBC,
    RAIL_RADIO,
    RAIL_GPS,
    RAIL_PAYLOAD_1,
    RAIL_PAYLOAD_2,
    RAIL_5V_BUS,
    RAIL_3V3_BUS,
    // TODO: add more rails as needed
} power_rail_t;

#endif
