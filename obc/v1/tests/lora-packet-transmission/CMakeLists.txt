cmake_minimum_required(VERSION 3.15)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR ARM)
set(CMAKE_CROSSCOMPILING TRUE)
set(CMAKE_SYSTEM_VERSION 1)

set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_AR arm-none-eabi-ar)
set(CMAKE_RANLIB arm-none-eabi-ranlib)
set(CMAKE_STRIP arm-none-eabi-strip)

set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)
set(CMAKE_ASM_COMPILER_WORKS 1)

set(PICO_BOARD pico_w)

include(pico_sdk_import.cmake)
project(lora-packet-transmission LANGUAGES C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

pico_sdk_init()

add_subdirectory(./lib/messaging ${CMAKE_CURRENT_BINARY_DIR}/osusat-build)

set(RFM69_LIB_PATH "./lib/rp2x_rfm69")
add_subdirectory(${RFM69_LIB_PATH} lib/rp2x_rfm69)

file(GLOB C_SOURCES "*.c" "src/*.c")
add_executable(lora-packet-transmission ${C_SOURCES})

target_include_directories(lora-packet-transmission PRIVATE
    ${PICO_SDK_PATH}/src/common
    ${PICO_SDK_PATH}/src/common/pico_stdlib/include
    ${PICO_SDK_PATH}/src/rp2_common/hardware_gpio/include
    ${PICO_SDK_PATH}/src/rp2_common/hardware_spi/include
    ${PICO_SDK_PATH}/src/common/pico_base/include
    .
)

target_link_libraries(lora-packet-transmission
    PRIVATE
        OSUSat::C
        rp2x_rfm69_lib
        pico_stdlib
        hardware_adc
        hardware_spi
        pico_cyw43_arch_none
)

target_compile_definitions(lora-packet-transmission PRIVATE RFM69_HIGH_POWER)

pico_enable_stdio_usb(lora-packet-transmission 1)
pico_enable_stdio_uart(lora-packet-transmission 1)
pico_add_extra_outputs(lora-packet-transmission)
