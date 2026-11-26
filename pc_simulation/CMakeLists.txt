cmake_minimum_required(VERSION 3.20)
project(DalhalPC LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_PARALLEL_LEVEL 8)

# ---------------------------------------
# Platform specific flags
# ---------------------------------------

if (WIN32)
    add_compile_definitions(_WIN32 _WIN32_WINNT=0x0A00)
    add_compile_options(-Wall -g -O0)
	#add_compile_options(-fsanitize=address,undefined -g -O1)
    set(EXECUTABLE_NAME test)
else()
    add_compile_options(-Wall -g -O0 -fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
    set(EXECUTABLE_NAME test)
endif()

# ---------------------------------------
# Include directories
# ---------------------------------------

set(REPO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/..)

# ports/common: platform independent Arduino core subset, shared by all non Arduino targets
set(PORTS_COMMON_DIRS
    ${REPO_ROOT}/ports/common/Arduino
    ${REPO_ROOT}/ports/common/WString
    ${REPO_ROOT}/ports/common/Print
    ${REPO_ROOT}/ports/common/IPAddress
    ${REPO_ROOT}/ports/common/PgmSpace
)

# ports/pc: implementations on top of the host OS (threads, sockets, files, iostream)
set(PORTS_PC_DIRS
    ${REPO_ROOT}/ports/pc/Arduino
    ${REPO_ROOT}/ports/pc/Stream
    ${REPO_ROOT}/ports/pc/Ticker
    ${REPO_ROOT}/ports/pc/LittleFS
    ${REPO_ROOT}/ports/pc/WiFi
    ${REPO_ROOT}/ports/pc/WiFiClient
    ${REPO_ROOT}/ports/pc/HTTPClient
    ${REPO_ROOT}/ports/pc/DALHAL_WebSocketAPI
)

# ports/pc/stubs: fake hardware drivers, replaced by real drivers on hardware targets
set(PORTS_PC_STUB_DIRS
    ${REPO_ROOT}/ports/pc/stubs/DHTesp
    ${REPO_ROOT}/ports/pc/stubs/OneWire
    ${REPO_ROOT}/ports/pc/stubs/DallasTemperature
    ${REPO_ROOT}/ports/pc/stubs/WS2812FX
    ${REPO_ROOT}/ports/pc/stubs/Adafruit_GFX
    ${REPO_ROOT}/ports/pc/stubs/Adafruit_SSD1306
    ${REPO_ROOT}/ports/pc/stubs/Wire
)

set(INCLUDE_DIRS
    ${REPO_ROOT}/src   # <-- THIS IS THE IMPORTANT ONE
    ${PORTS_COMMON_DIRS}
    ${PORTS_PC_DIRS}
    ${PORTS_PC_STUB_DIRS}
    ${REPO_ROOT}/.pio/libdeps/esp32dev/ArduinoJson/src
    ${REPO_ROOT}/.pio/libdeps/esp32dev/Time
    ${REPO_ROOT}/.pio/libdeps/esp32dev/TimeAlarms
    ${REPO_ROOT}/lib/PubSubClient/src
)

# ---------------------------------------
# Source discovery
# ---------------------------------------

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/main_pc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/commandLoop.cpp
    ${REPO_ROOT}/src/*.cpp
    ${REPO_ROOT}/ports/common/*.cpp
    ${REPO_ROOT}/ports/pc/*.cpp
    ${REPO_ROOT}/lib/PubSubClient/src/PubSubClient.cpp
    ${REPO_ROOT}/lib/PubSubClient/src/PubSubClient_ErrorStrings.cpp
    ${REPO_ROOT}/.pio/libdeps/esp32dev/Time/Time.cpp
    ${REPO_ROOT}/.pio/libdeps/esp32dev/TimeAlarms/TimeAlarms.cpp
)

# Remove CMake-generated files
list(FILTER SOURCES EXCLUDE REGEX "CMakeFiles|cmake_")

# ---------------------------------------
# Exclude specific files
# ---------------------------------------

list(FILTER SOURCES EXCLUDE REGEX ".*DALHAL_REST\\.cpp$")
list(FILTER SOURCES EXCLUDE REGEX ".*/DALHAL\\.cpp$")

# ---------------------------------------
# Create executable
# ---------------------------------------

add_executable(${EXECUTABLE_NAME} ${SOURCES})

# ---------------------------------------
# Generate embedded frontend files
# ---------------------------------------

find_package(Python3 REQUIRED COMPONENTS Interpreter)
set(GEN_FRONTEND ${CMAKE_CURRENT_SOURCE_DIR}/../tools/gen_frontend.py)
# run once at configure time so the header exists for the first build
execute_process(COMMAND ${Python3_EXECUTABLE} ${GEN_FRONTEND})
# and before every build, the script only rewrites the header when www/ content changed
add_custom_target(frontend_files COMMAND ${Python3_EXECUTABLE} ${GEN_FRONTEND})
add_dependencies(${EXECUTABLE_NAME} frontend_files)


target_include_directories(${EXECUTABLE_NAME} PRIVATE ${INCLUDE_DIRS})

# ---------------------------------------
# Windows specific libs
# ---------------------------------------

set_target_properties(${EXECUTABLE_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/.."
)

if (WIN32)
    target_link_libraries(${EXECUTABLE_NAME} PRIVATE winhttp iphlpapi ws2_32)
	target_link_libraries(test PRIVATE bcrypt ws2_32)
endif()