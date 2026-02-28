cmake_minimum_required(VERSION 3.20)
project(DalhalPC LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------------------------------
# Platform specific flags
# ---------------------------------------

if (WIN32)
    add_compile_definitions(_WIN32 _WIN32_WINNT=0x0A00)
    add_compile_options(-Wall -g -O0)
    set(EXECUTABLE_NAME test.exe)
else()
    add_compile_options(-Wall -g -O0 -fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
    set(EXECUTABLE_NAME test)
endif()

# ---------------------------------------
# Include directories
# ---------------------------------------

set(INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/stubs/Arduino
    ${CMAKE_CURRENT_SOURCE_DIR}/ports/IPAddress
    ${CMAKE_CURRENT_SOURCE_DIR}/stubs/PrintStream
    ${CMAKE_CURRENT_SOURCE_DIR}/cpp-httplib
    ${CMAKE_CURRENT_SOURCE_DIR}/../.pio/libdeps/esp32dev/ArduinoJson/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/PubSubClient/src
    ${CMAKE_CURRENT_SOURCE_DIR}/ports/Ticker
    ${CMAKE_CURRENT_SOURCE_DIR}/ports/LittleFS
    ${CMAKE_CURRENT_SOURCE_DIR}/ports/WiFiClient
    ${CMAKE_CURRENT_SOURCE_DIR}/ports/HTTPClient
    ${CMAKE_CURRENT_SOURCE_DIR}/ports/WiFi
    ${CMAKE_CURRENT_SOURCE_DIR}/stubs/DHTesp
    ${CMAKE_CURRENT_SOURCE_DIR}/stubs/OneWire
    ${CMAKE_CURRENT_SOURCE_DIR}/stubs/DallasTemperature
    ${CMAKE_CURRENT_SOURCE_DIR}/stubs/WS2812FX
    ${CMAKE_CURRENT_SOURCE_DIR}/stubs/Adafruit_GFX
    ${CMAKE_CURRENT_SOURCE_DIR}/stubs/Adafruit_SSD1306
    ${CMAKE_CURRENT_SOURCE_DIR}/stubs/Wire
)

# ---------------------------------------
# Recursive source discovery
# ---------------------------------------

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    *.cpp
    ../src/*.cpp
)

# ---------------------------------------
# Exclude specific files
# ---------------------------------------

list(FILTER SOURCES EXCLUDE REGEX ".*DALHAL_REST\\.cpp$")
list(FILTER SOURCES EXCLUDE REGEX ".*/DALHAL\\.cpp$")

# ---------------------------------------
# Create executable
# ---------------------------------------

add_executable(${EXECUTABLE_NAME} ${SOURCES})

target_include_directories(${EXECUTABLE_NAME} PRIVATE ${INCLUDE_DIRS})

# ---------------------------------------
# Windows specific libs
# ---------------------------------------

if (WIN32)
    target_link_libraries(${EXECUTABLE_NAME} PRIVATE winhttp ws2_32)
endif()