cmake_minimum_required(VERSION 3.16)
project(oven LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------------------------------------------------------------------------
# Collect all source files automatically
# ---------------------------------------------------------------------------
file(GLOB_RECURSE OVEN_SOURCES
    CONFIGURE_DEPENDS
    src/*.cpp
)

add_executable(oven ${OVEN_SOURCES})

# ---------------------------------------------------------------------------
# Include directories (for headers in src/)
# ---------------------------------------------------------------------------
target_include_directories(oven PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ---------------------------------------------------------------------------
# Link required libraries
# ---------------------------------------------------------------------------
find_package(PkgConfig REQUIRED)

# GPIO (for your heater/fan code)
pkg_check_modules(GPIOD REQUIRED libgpiod)
target_include_directories(oven PRIVATE ${GPIOD_INCLUDE_DIRS})
target_link_libraries(oven PRIVATE ${GPIOD_LIBRARIES})

# MODBUS (for RS-485 THKA controller)
pkg_check_modules(LIBMODBUS REQUIRED libmodbus)
target_include_directories(oven PRIVATE ${LIBMODBUS_INCLUDE_DIRS})
target_link_libraries(oven PRIVATE ${LIBMODBUS_LIBRARIES})

# ---------------------------------------------------------------------------
# Compiler warnings and optimization (optional but recommended)
# ---------------------------------------------------------------------------
target_compile_options(oven PRIVATE -Wall -Wextra -Wpedantic)
