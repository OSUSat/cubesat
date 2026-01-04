cmake_minimum_required(VERSION 3.15)
project(eps_firmware C ASM)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)

add_library(eps_headers INTERFACE)

target_include_directories(eps_headers INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/app
    ${CMAKE_CURRENT_SOURCE_DIR}/services
    ${CMAKE_CURRENT_SOURCE_DIR}/hal
    ${CMAKE_CURRENT_SOURCE_DIR}/config
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks
)

set(TARGET_ARCH "ARM" CACHE STRING "Target architecture (ARM or HOST)")
message(STATUS "Target architecture: ${TARGET_ARCH}")

if(TARGET_ARCH STREQUAL "ARM")
    message(STATUS "Building for ARM target")
    set(MCU_FLAGS "-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${MCU_FLAGS} -fdata-sections -ffunction-sections -Wall")
    set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${MCU_FLAGS}")

    set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/bsp/startup/STM32L496XX_FLASH.ld")
    set(CMAKE_EXE_LINKER_FLAGS "${MCU_FLAGS} -specs=nosys.specs -specs=nano.specs -T${LINKER_SCRIPT} -Wl,-Map=${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map,--cref -Wl,--gc-sections")
else()
    message(STATUS "Building for HOST target")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
endif()

option(BUILD_HITL "Build with simulated hardware mocks" OFF)
if(TARGET_ARCH STREQUAL "HOST")
    set(BUILD_HITL ON)
endif()

add_subdirectory(./lib/messaging ${CMAKE_CURRENT_BINARY_DIR}/osusat-messaging-build)
add_subdirectory(./lib/core ${CMAKE_CURRENT_BINARY_DIR}/osusat-core-build)

file(GLOB APP_SRC app/*.c)
file(GLOB SERVICES_SRC services/*.c)

if(TARGET_ARCH STREQUAL "ARM")
     file(GLOB HAL_SRC
        hal/*.c
        bsp/hal_driver/Src/*.c
        bsp/mcu/Src/*.c
        bsp/system/*.c
        bsp/startup/*.c
        bsp/startup/*.s
    )

    add_executable(eps_firmware
        main.c
        ${APP_SRC}
        ${SERVICES_SRC}
        ${HAL_SRC}
    )

    target_include_directories(eps_firmware PRIVATE
        bsp/cmsis/Include
        bsp/cmsis/Device/ST/STM32L4xx/Include
        bsp/hal_driver/Inc
        bsp/mcu/Inc
        bsp/system
        hal
        app
        services
        config
    )

    target_compile_definitions(eps_firmware PRIVATE
        STM32L496xx
        USE_HAL_DRIVER
    )

elseif(BUILD_HITL)
    message(STATUS "Building for HITL testing...")
    add_definitions(-DHITL)

    set(HITL_HAL_MOCKS
        mocks/bsp/hal_stubs.c
        mocks/hal_adc_mock.c
        mocks/hal_gpio_mock.c
        mocks/hal_uart_mock.c
        mocks/hal_i2c_mock.c
        mocks/bsp/clock_mock.c
    )

    add_executable(eps_firmware
        main.c
        ${APP_SRC}
        ${SERVICES_SRC}
        ${HITL_HAL_MOCKS}
    )

    include_directories(
        mocks/bsp
        hal
        mocks
        app
        services
        config
    )
endif()

enable_testing()
add_subdirectory(tests)

target_link_libraries(eps_firmware PRIVATE
    eps_headers
    OSUSat::C
    OSUSat::Core
)

if(TARGET_ARCH STREQUAL "ARM")
    set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
    set(CMAKE_SIZE arm-none-eabi-size)

    add_custom_command(TARGET eps_firmware POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:eps_firmware> ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.hex
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:eps_firmware> ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.bin
        COMMAND ${CMAKE_SIZE} $<TARGET_FILE:eps_firmware>
        COMMENT "Building ${PROJECT_NAME}.hex and ${PROJECT_NAME}.bin"
    )
endif()

find_package(Doxygen REQUIRED COMPONENTS doxygen)
find_program(SPHINX_BUILD sphinx-build)

if (DOXYGEN_FOUND)
    set(DOXYFILE ${CMAKE_CURRENT_SOURCE_DIR}/../documentation/doxygen/Doxyfile)

    add_custom_target(doxygen
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../documentation/doxygen
        COMMENT "Generating Doxygen XML API documentation"
        VERBATIM
    )

    if (SPHINX_BUILD)
        add_custom_target(sphinx
            COMMAND ${SPHINX_BUILD}
                    -b html
                    ${CMAKE_CURRENT_SOURCE_DIR}/../documentation/sphinx/source
                    ${CMAKE_CURRENT_BINARY_DIR}/documentation/sphinx
            COMMENT "Generating Sphinx documentation"
            VERBATIM
        )

        add_dependencies(sphinx doxygen)

        add_custom_target(docs
            DEPENDS doxygen sphinx
        )
    endif()
endif()
