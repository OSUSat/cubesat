#include "config/eps_power_profiles.h"
#include "services/power_profiles.h"
#include "tests/mocks/rail_controller_mock.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

void test_enable_nominal_profile(void) {
    printf("Running test: %s\n", __func__);

    mock_reset_rail_controller();
    rail_controller_t controller;

    power_profile_status_t status =
        power_profiles_enable(&controller, POWER_PROFILE_NOMINAL);

    assert(status == POWER_PROFILE_SUCCESS);

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

    power_profile_status_t status =
        power_profiles_disable(&controller, POWER_PROFILE_SAFE);

    assert(status == POWER_PROFILE_SUCCESS);

    int expected_count = sizeof(safe_mode_rails) / sizeof(safe_mode_rails[0]);

    assert(mock_get_disabled_count() == expected_count);

    for (int i = 0; i < expected_count; i++) {
        assert(mock_get_disabled_rail(i) == safe_mode_rails[i]);
    }

    printf("Test passed.\n");
}

void test_invalid_profile(void) {
    printf("Running test: %s\n", __func__);

    mock_reset_rail_controller();
    rail_controller_t controller;

    // POWER_PROFILE_SAFE + 1 is an invalid profile
    power_profile_status_t status =
        power_profiles_enable(&controller, POWER_PROFILE_SAFE + 1);

    assert(status == POWER_PROFILE_ERROR_INVALID_PROFILE);
    assert(mock_get_enabled_count() == 0);

    status = power_profiles_disable(&controller, POWER_PROFILE_SAFE + 1);

    assert(status == POWER_PROFILE_ERROR_INVALID_PROFILE);
    assert(mock_get_disabled_count() == 0);

    printf("Test passed.\n");
}

int main(void) {
    test_enable_nominal_profile();
    test_disable_safe_profile();
    test_invalid_profile();

    return 0;
}
