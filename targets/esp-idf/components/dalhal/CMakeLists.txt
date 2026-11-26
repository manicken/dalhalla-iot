# DALHAL core as an ESP-IDF component, sources come from the shared src/ and ports/ folders
get_filename_component(REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../.." ABSOLUTE)

# the parts of src/ that are built on ESP-IDF so far (step 3b), the rest follows when its ports exist
set(DALHAL_SRC_DIRS
    src/DALHAL/Core
    src/DALHAL/ScriptEngine
    src/DALHAL/Support
    src/DALHAL/Devices/Script
    src/DALHAL/Devices/GeneralInputs
    src/DALHAL/Devices/GeneralOutputs
    src/DALHAL/Devices/DeviceContainer
    src/DALHAL/Devices/Actuators
    src/DALHAL/Devices/RF433
    src/DALHAL/Devices/_Registry
    src/DALHAL/Devices/Template
    src/System/Platform
    ports/common
    ports/esp-idf
)

set(DALHAL_SRCS)
foreach(dir ${DALHAL_SRC_DIRS})
    # no CONFIGURE_DEPENDS, ESP-IDF also evaluates this file in script mode, new files need a reconfigure
    file(GLOB_RECURSE dir_srcs "${REPO_ROOT}/${dir}/*.cpp")
    list(APPEND DALHAL_SRCS ${dir_srcs})
endforeach()
# PC only test code
list(FILTER DALHAL_SRCS EXCLUDE REGEX ".*_Parser_Tests\\.cpp$")
# single files from folders that are not built as a whole yet
list(APPEND DALHAL_SRCS
    "${REPO_ROOT}/src/Support/LittleFS_ext.cpp"
    "${REPO_ROOT}/src/DALHAL/API/DALHAL_StringBuilderStreamer.cpp"
    "${REPO_ROOT}/src/DALHAL/API/DALHAL_BlockStreamer.cpp"
    "${REPO_ROOT}/src/DALHAL/Drivers/RF433.cpp"
)

# ports/common + ports/esp-idf provide the Arduino core subset, each folder is a header root
file(GLOB PORT_DIRS LIST_DIRECTORIES true "${REPO_ROOT}/ports/common/*" "${REPO_ROOT}/ports/esp-idf/*")

idf_component_register(
    SRCS ${DALHAL_SRCS}
    INCLUDE_DIRS
        "${REPO_ROOT}/src"
        ${PORT_DIRS}
        # header only, same copy as the PC CMake build uses
        "${REPO_ROOT}/.pio/libdeps/esp32dev/ArduinoJson/src"
    REQUIRES driver esp_timer esp_system freertos
)

# ESP32 = chip family, as the Arduino core defines it. DALHAL_BACKEND_ESPIDF comes from Platform.h
# (ESP_PLATFORM without ARDUINO) and tells the Arduino core and the ESP-IDF backend apart.
target_compile_definitions(${COMPONENT_LIB} PUBLIC ESP32)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-unused-variable -Wno-unused-function)
