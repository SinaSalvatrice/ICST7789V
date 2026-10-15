cmake_minimum_required(VERSION 3.13)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(st7789_red_test C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(st7789_red_test
    main.c
)

target_link_libraries(st7789_red_test
    pico_stdlib
    hardware_spi
)

pico_enable_stdio_usb(st7789_red_test 0)
pico_enable_stdio_uart(st7789_red_test 0)
pico_add_extra_outputs(st7789_red_test)
