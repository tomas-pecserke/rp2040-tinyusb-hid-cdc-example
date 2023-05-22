cmake_minimum_required(VERSION 3.25)

# Fetch Pico SDK locally
set(PICO_SDK_FETCH_FROM_GIT ON)

# Fetch Pico Extras locally
set(PICO_EXTRAS_FETCH_FROM_GIT ON)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

# Pull in Pico Extras (Optional)
include(pico_extras_import.cmake)

project(rp2040-tinyusb-hid-cdc-example C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

set(PROJECT_EXECUTABLE "rp2040-tinyusb-hid-cdc-example")
set(PROJECT_PATH ${PROJECT_SOURCE_DIR})
set(PROJECT_URL_BASE "https://github.com/<author>/<repo>/tree/HEAD")

set(FAMILY_MCUS RP2040)

# Initialize the SDK
pico_sdk_init()

include(program_auto_set_url.cmake)

add_compile_options(
    -Wall
    -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
    -Wno-unused-function # we have some for the docs that aren't called
    -Wno-maybe-uninitialized
)

add_executable(${PROJECT_EXECUTABLE})

FILE(GLOB SOURCES ${CMAKE_SOURCE_DIR}/src/*.c ${CMAKE_SOURCE_DIR}/src/*.cpp)
target_sources(${PROJECT_EXECUTABLE} PUBLIC ${SOURCES})

target_include_directories(${PROJECT_EXECUTABLE} PUBLIC ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/include)

# Pull in common dependencies
target_link_libraries(
    ${PROJECT_EXECUTABLE}
    pico_stdlib
    pico_stdio_usb
    pico_unique_id
    pico_usb_reset_interface
    tinyusb_device
)

# Create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(${PROJECT_EXECUTABLE})

# Add url via pico_set_program_url
program_auto_set_url(${PROJECT_EXECUTABLE})
