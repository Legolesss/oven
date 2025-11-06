# ---------------------- toolchain / project basics --------------------------
cmake_minimum_required(VERSION 3.16)
project(oven LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------------------------- find Qt6 modules ------------------------------
find_package(Qt6 REQUIRED COMPONENTS Quick Qml)

# ---------------------- system libs (your existing ones) --------------------
# Use a single PkgConfig call and reuse results for both targets.
find_package(PkgConfig REQUIRED)

# libgpiod (GPIO)
pkg_check_modules(GPIOD REQUIRED libgpiod)

# libmodbus (RS-485)
pkg_check_modules(LIBMODBUS REQUIRED libmodbus)

# ----------------------------- source files ---------------------------------
# Glob everything under src/, then exclude the scanner utility from the main app.
file(GLOB_RECURSE OVEN_SOURCES
     CONFIGURE_DEPENDS
     src/*.cpp)

# Exclude src/thka_sv_scan.cpp from the main app
list(FILTER OVEN_SOURCES EXCLUDE REGEX ".*/thka_sv_scan\\.cpp$")

# Create the oven executable from the filtered sources
add_executable(oven ${OVEN_SOURCES})

# Headers in your repo
target_include_directories(oven PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/src/data
  ${GPIOD_INCLUDE_DIRS}
  ${LIBMODBUS_INCLUDE_DIRS}
)

# Link libs
target_link_libraries(oven PRIVATE
  Qt6::Quick
  Qt6::Qml
  ${GPIOD_LIBRARIES}
  ${LIBMODBUS_LIBRARIES}
  stdc++fs  # For std::filesystem support
)

# Helpful warnings
target_compile_options(oven PRIVATE -Wall -Wextra -Wpedantic)

# --------------------------- Qt QML resources -------------------------------
qt_add_qml_module(oven
  URI OVEN
  VERSION 1.0
  QML_FILES
    qml/Main.qml
)