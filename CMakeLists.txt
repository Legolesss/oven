# ---------------------- toolchain / project basics --------------------------
cmake_minimum_required(VERSION 3.16)                # require a modern-enough CMake
project(oven LANGUAGES CXX)                         # project name + C++ language only

set(CMAKE_AUTOMOC ON)     # <-- enables Qt's automatic meta-object compiler
set(CMAKE_AUTORCC ON)     # <-- optional, embeds qrc:/ resources like QML/images
set(CMAKE_CXX_STANDARD 20)                          # use C++20 (matches your code)
set(CMAKE_CXX_STANDARD_REQUIRED ON)                 # fail if the compiler can't do C++20
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)               # emit compile_commands.json (helpful for IDEs)

# ---------------------------- find Qt6 modules ------------------------------
# We use Qt Quick (QML engine) to render the UI. Qml is the build-time helper.
find_package(Qt6 REQUIRED COMPONENTS Quick Qml)     # hard-require the Qt6 Quick + Qml packages

# If/when you add these features later, just uncomment:
# find_package(Qt6 REQUIRED COMPONENTS SerialBus SerialPort Sql Charts)

# ----------------------------- source files ---------------------------------
# Keep your pattern: grab all .cpp files under src/ (includes our new ui/*.cpp)
file(GLOB_RECURSE OVEN_SOURCES
     CONFIGURE_DEPENDS
     src/*.cpp)

# Create the oven executable from the collected sources
add_executable(oven ${OVEN_SOURCES})

# Tell the target where to find headers in your repo (src/, plus subfolders)
target_include_directories(oven PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# --------------------------- Qt: link + QML pack ----------------------------
# Link the Qt6 libraries we found above into the oven binary
target_link_libraries(oven PRIVATE Qt6::Quick Qt6::Qml)

# Package the QML files into the binary via Qt's resource system.
# - URI is a logical import path; not a URL. We won't import by URI yet,
#   but this ensures qml/Main.qml is embedded and loadable as "qrc:/qml/Main.qml".
qt_add_qml_module(oven
  URI OVEN                                   # arbitrary module name for tooling
  VERSION 1.0                                # arbitrary version
  QML_FILES
    qml/Main.qml                             # the top-level QML file you created

)

# ---------------------- system libs (your existing ones) --------------------
# You already use libgpiod (GPIO) and libmodbus (RS-485/Modbus). Keep them.
find_package(PkgConfig REQUIRED)                   # use pkg-config to locate C libs

# libgpiod (GPIO control for fan/heater boards)
pkg_check_modules(GPIOD REQUIRED libgpiod)         # locate headers & libs
target_include_directories(oven PRIVATE ${GPIOD_INCLUDE_DIRS})  # add include path
target_link_libraries(oven PRIVATE ${GPIOD_LIBRARIES})          # link libgpiod

# libmodbus (RS-485 THKA controller comms)
pkg_check_modules(LIBMODBUS REQUIRED libmodbus)    # locate headers & libs
target_include_directories(oven PRIVATE ${LIBMODBUS_INCLUDE_DIRS}) # add include path
target_link_libraries(oven PRIVATE ${LIBMODBUS_LIBRARIES})         # link libmodbus

# ---------------------- compiler warnings / hardening -----------------------
# Helpful warnings while you iterate
target_compile_options(oven PRIVATE -Wall -Wextra -Wpedantic)
