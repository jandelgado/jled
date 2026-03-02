cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Pull in Pico SDK (must be before project) - set PICO_SDK_PATH env var
include(pico_sdk_import.cmake)

project(pico_demo C CXX ASM)

# Initialise the Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1
add_executable(pico_demo pico_demo.cpp )
pico_set_program_name(pico_demo "pico_demo")
pico_set_program_version(pico_demo "0.1")

pico_enable_stdio_uart(pico_demo 1)
pico_enable_stdio_usb(pico_demo 0)

# Add the standard library to the build
target_link_libraries(pico_demo pico_stdlib hardware_pwm JLed)

pico_add_extra_outputs(pico_demo)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../..src)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../.. build)

