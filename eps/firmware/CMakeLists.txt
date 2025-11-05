cmake_minimum_required(VERSION 3.15)
project(eps_firmware C ASM)
set(CMAKE_C_STANDARD 11)

set(MCU_FLAGS "-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${MCU_FLAGS} -fdata-sections -ffunction-sections -Wall")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${MCU_FLAGS}")

set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/STM32L496ZGTx_FLASH.ld")
set(CMAKE_EXE_LINKER_FLAGS "${MCU_FLAGS} -specs=nosys.specs -specs=nano.specs -T${LINKER_SCRIPT} -Wl,-Map=${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map,--cref -Wl,--gc-sections")

option(BUILD_HITL "Build with simulated hardware mocks" OFF)

add_subdirectory(./lib/messaging ${CMAKE_CURRENT_BINARY_DIR}/osusat-build)

include_directories(app services hal config)

file(GLOB APP_SRC app/*.c)
file(GLOB SERVICES_SRC services/*.c)
file(GLOB HAL_SRC hal/*.c)
file(GLOB MOCKS_SRC mocks/*.c)

if(BUILD_HITL)
    message(STATUS "Building for HITL testing...")
    add_definitions(-DHITL)
    add_executable(eps_firmware 
        main.c 
        ${APP_SRC} 
        ${SERVICES_SRC} 
        ${MOCKS_SRC}
    )
else()
    message(STATUS "Building for hardware target...")
    add_executable(eps_firmware 
        main.c 
        startup_stm32l496xx.s
        system_stm32l4xx.c
        ${APP_SRC} 
        ${SERVICES_SRC} 
        ${HAL_SRC}
    )
endif()

enable_testing()
add_subdirectory(tests)

target_include_directories(eps_firmware PRIVATE
    OSUSat::C
)

set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
set(CMAKE_SIZE arm-none-eabi-size)

add_custom_command(TARGET eps_firmware POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:eps_firmware> ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:eps_firmware> ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.bin
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:eps_firmware>
    COMMENT "Building ${PROJECT_NAME}.hex and ${PROJECT_NAME}.bin"
)
