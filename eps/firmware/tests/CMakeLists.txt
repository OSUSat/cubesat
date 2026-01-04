cmake_minimum_required(VERSION 3.15)

set(TEST_COMMON_LIBS
    eps_headers
    OSUSat::Core
    OSUSat::C
)

include_directories(
    ../
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks
)

# test_power_profiles

add_executable(test_power_profiles
    test_power_profiles.c
    mocks/rail_controller_mock.c
    mocks/event_bus_mock.c
    ../services/power_profiles.c
)

target_link_libraries(test_power_profiles PRIVATE
    ${TEST_COMMON_LIBS}
)

add_test(
    NAME test_power_profiles
    COMMAND test_power_profiles
)

# test_adc_mock

add_executable(test_adc_mock
    test_adc_mock.c
    ../mocks/adc_mock.c
)

target_link_libraries(test_adc_mock PRIVATE
    ${TEST_COMMON_LIBS}
)

add_test(
    NAME test_adc_mock
    COMMAND test_adc_mock
)

# test_gpio_mock

add_executable(test_gpio_mock
    test_gpio_mock.c
    ../mocks/gpio_mock.c
)

target_link_libraries(test_gpio_mock PRIVATE
    ${TEST_COMMON_LIBS}
)

add_test(
    NAME test_gpio_mock
    COMMAND test_gpio_mock
)

# test_uart_mock

add_executable(test_uart_mock
    test_uart_mock.c
    ../mocks/uart_mock.c
)

target_link_libraries(test_uart_mock PRIVATE
    ${TEST_COMMON_LIBS}
)

add_test(
    NAME test_uart_mock
    COMMAND test_uart_mock
)

# test_battery_management

add_executable(test_battery_management
    test_battery_management.c
    ../services/battery_management.c
    mocks/event_bus_mock.c
)

target_link_libraries(test_battery_management PRIVATE
    ${TEST_COMMON_LIBS}
)

add_test(
    NAME test_battery_management
    COMMAND test_battery_management
)
