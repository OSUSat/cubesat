#include "app/events.h"
#include "config/eps_power_profiles.h"
#include "osusat/event_bus.h"
#include "services/power_profiles.h"
#include "tests/mocks/rail_controller_mock.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

void test_enable_nominal_profile(void) {
    printf("Running test: %s\n", __func__);

    mock_reset_rail_controller();
    rail_controller_t controller;
    power_profiles_t profiles;
    power_profiles_init(&profiles, &controller);

    // manually trigger the event that would be sent by the power_policies app
    osusat_event_bus_publish(APP_EVENT_REQUEST_POWER_PROFILE_NOMINAL, NULL, 0);
    // in the test environment, we need to manually process the event
    osusat_event_bus_process();

    int expected_count =
        sizeof(nominal_mode_rails) / sizeof(nominal_mode_rails[0]);

    assert(mock_get_enabled_count() == expected_count);

    for (int i = 0; i < expected_count; i++) {
        assert(mock_get_enabled_rail(i) == nominal_mode_rails[i]);
    }

    printf("Test passed.\n");
}

void test_disable_safe_profile(void) {
    printf("Running test: %s\n", __func__);

    mock_reset_rail_controller();
    rail_controller_t controller;
    power_profiles_t profiles;
    power_profiles_init(&profiles, &controller);

    // the service starts in SAFE mode, so switch to NOMINAL first
    osusat_event_bus_publish(APP_EVENT_REQUEST_POWER_PROFILE_NOMINAL, NULL, 0);
    osusat_event_bus_process();
    mock_reset_rail_controller(); // reset mock counts after the switch

    // now request SAFE mode, which should trigger the disable of NOMINAL rails
    osusat_event_bus_publish(APP_EVENT_REQUEST_POWER_PROFILE_SAFE, NULL, 0);
    osusat_event_bus_process();

    int expected_count =
        sizeof(nominal_mode_rails) / sizeof(nominal_mode_rails[0]);

    assert(mock_get_disabled_count() == expected_count);

    for (int i = 0; i < expected_count; i++) {
        assert(mock_get_disabled_rail(i) == nominal_mode_rails[i]);
    }

    printf("Test passed.\n");
}

void test_invalid_profile(void) {
    printf("Running test: %s\n", __func__);

    mock_reset_rail_controller();
    rail_controller_t controller;
    power_profiles_t profiles;
    power_profiles_init(&profiles, &controller);

    // try to enable an invalid profile
    power_profile_status_t status =
        power_profiles_enable(&profiles, POWER_PROFILE_SAFE + 1);

    assert(status == POWER_PROFILE_ERROR_INVALID_PROFILE);
    assert(mock_get_enabled_count() == 0);

    // try to disable an invalid profile
    status = power_profiles_disable(&profiles, POWER_PROFILE_SAFE + 1);

    assert(status == POWER_PROFILE_ERROR_INVALID_PROFILE);
    assert(mock_get_disabled_count() == 0);

    printf("Test passed.\n");
}

int main(void) {
    osusat_event_bus_init(NULL, 0);
    test_enable_nominal_profile();
    test_disable_safe_profile();
    test_invalid_profile();

    return 0;
}
