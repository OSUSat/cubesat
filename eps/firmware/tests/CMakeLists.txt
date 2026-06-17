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

if(TARGET_ARCH STREQUAL "ARM")
    include_directories(
        ../bsp/mcu/Inc
        ../bsp/hal_driver/Inc
        ../bsp/cmsis/Include
        ../bsp/cmsis/Device/ST/STM32L4xx/Include
    )
else()
    include_directories(
        ../mocks/bsp
    )
endif()

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
    ../mocks/hal_adc_mock.c
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
    ../mocks/hal_gpio_mock.c
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
    ../mocks/hal_uart_mock.c
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

# test_telemetry

add_executable(test_telemetry
    test_telemetry.c
    ../services/telemetry.c
)

target_link_libraries(test_telemetry PRIVATE
    ${TEST_COMMON_LIBS}
)

add_test(
    NAME test_telemetry
    COMMAND test_telemetry
)
# test_watchdog

if(NOT TARGET_ARCH STREQUAL "ARM")
    add_executable(test_watchdog
        test_watchdog.c
        ../services/watchdog.c
        ../mocks/bsp/hal_stubs.c
        ../mocks/hal_time_mock.c
        ../mocks/hal_gpio_mock.c
    )

    target_link_libraries(test_watchdog PRIVATE
        ${TEST_COMMON_LIBS}
    )

    add_test(
        NAME test_watchdog
        COMMAND test_watchdog
    )
endif()

# test_can_mock

if(NOT TARGET_ARCH STREQUAL "ARM")
    add_executable(test_can_mock
        test_can_mock.c
        ../mocks/hal_can_mock.c
    )

    target_link_libraries(test_can_mock PRIVATE
        ${TEST_COMMON_LIBS}
    )

    add_test(
        NAME test_can_mock
        COMMAND test_can_mock
    )
endif()

# test_can_events

if(NOT TARGET_ARCH STREQUAL "ARM")
    add_executable(test_can_events
        test_can_events.c
        ../services/can_events.c
        ../mocks/hal_can_mock.c
        mocks/event_bus_mock.c
    )

    target_link_libraries(test_can_events PRIVATE
        ${TEST_COMMON_LIBS}
    )

    add_test(
        NAME test_can_events
        COMMAND test_can_events
    )
endif()

# test_logging

if(NOT TARGET_ARCH STREQUAL "ARM")
    add_executable(test_logging
        test_logging.c
        ../services/logging.c
        ../services/can_events.c
        ../mocks/hal_can_mock.c
        ../mocks/hal_flash_mock.c
        mocks/event_bus_mock.c
    )

    target_link_libraries(test_logging PRIVATE
        ${TEST_COMMON_LIBS}
    )

    add_test(
        NAME test_logging
        COMMAND test_logging
    )
endif()

