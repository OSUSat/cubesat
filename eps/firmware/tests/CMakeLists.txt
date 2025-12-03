include_directories(../)

add_executable(test_power_profiles
    test_power_profiles.c
    mocks/rail_controller_mock.c
    ../services/power_profiles.c
)

add_test(
    NAME test_power_profiles
    COMMAND test_power_profiles
)

add_executable(test_adc_mock
    test_adc_mock.c
    ../mocks/adc_mock.c
)

add_test(
    NAME test_adc_mock
    COMMAND test_adc_mock
)

add_executable(test_gpio_mock
    test_gpio_mock.c
    ../mocks/gpio_mock.c
)

add_test(
    NAME test_gpio_mock
    COMMAND test_gpio_mock
)

add_executable(test_uart_mock
    test_uart_mock.c
    ../mocks/uart_mock.c
)
target_link_libraries(test_uart_mock PRIVATE OSUSat::Core)

add_test(
    NAME test_uart_mock
    COMMAND test_uart_mock
)

add_executable(test_battery_management
    test_battery_management.c
    ../services/battery_management.c
    mocks/event_bus_mock.c
)
target_link_libraries(test_battery_management PRIVATE OSUSat::Core)

add_test(
    NAME test_battery_management
    COMMAND test_battery_management
)
