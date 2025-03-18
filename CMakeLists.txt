cmake_minimum_required(VERSION 3.13)

if(NOT DEFINED PICO_SDK_PATH)
    message(FATAL_ERROR
            "PICO_SDK_PATH was not specified.
             pico_sdk_init() must first be invoked.")
endif()

project(rp2040_pwm)

# Use modern conventions like std::invoke
set(CMAKE_CXX_STANDARD 17)

add_library(rp2040_pwm
    src/pwm.cpp
)

# Header file location exposed with target scope for external projects.
target_include_directories(rp2040_pwm PUBLIC inc)

target_link_libraries(rp2040_pwm hardware_pwm pico_stdlib)

if(DEBUG)
    # UART stdio for printf.
    pico_enable_stdio_uart(pwm 1)
endif()
