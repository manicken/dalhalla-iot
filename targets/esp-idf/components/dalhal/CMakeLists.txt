# DALHAL core as an ESP-IDF component, sources come from the shared src/ and ports/ folders
get_filename_component(REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../.." ABSOLUTE)

# the same library copies as the PC CMake build (installed by building the root esp32dev env once)
set(LIBDEPS "${REPO_ROOT}/.pio/libdeps/esp32dev")

# the parts of src/ that are built on ESP-IDF so far, the rest follows when its ports exist
set(DALHAL_SRC_DIRS
    src/DALHAL/API
    src/DALHAL/Core
    src/DALHAL/ScriptEngine
    src/DALHAL/Support
    src/DALHAL/Devices/Script
    src/DALHAL/Devices/GeneralInputs
    src/DALHAL/Devices/GeneralOutputs
    src/DALHAL/Devices/DeviceContainer
    src/DALHAL/Devices/HomeAssistant
    src/DALHAL/Devices/Actuators
    src/DALHAL/Devices/RF433
    src/DALHAL/Devices/_Registry
    src/DALHAL/Devices/Template
    src/Scheduler
    src/System/Network
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
    "${REPO_ROOT}/src/DALHAL/DALHAL.cpp"
    "${REPO_ROOT}/src/DALHAL/Drivers/RF433.cpp"
    "${REPO_ROOT}/src/DALHAL/Drivers/HeartbeatLed.cpp"
    "${REPO_ROOT}/src/Support/LittleFS_ext.cpp"
    "${REPO_ROOT}/src/Support/NTP.cpp"
    "${REPO_ROOT}/src/Support/mimetable.cpp"
    "${REPO_ROOT}/src/Support/Time_ext.cpp"
    "${REPO_ROOT}/src/System/WiFiConnectionManager.cpp"
    "${REPO_ROOT}/src/System/DeviceUID.cpp"
    "${REPO_ROOT}/src/System/Info.cpp"
    "${REPO_ROOT}/lib/PubSubClient/src/PubSubClient.cpp"
    "${REPO_ROOT}/lib/PubSubClient/src/PubSubClient_ErrorStrings.cpp"
    "${LIBDEPS}/Time/Time.cpp"
    "${LIBDEPS}/TimeAlarms/TimeAlarms.cpp"
)

# ports/common + ports/esp-idf provide the Arduino core subset, each folder is a header root
file(GLOB PORT_DIRS LIST_DIRECTORIES true "${REPO_ROOT}/ports/common/*" "${REPO_ROOT}/ports/esp-idf/*")

idf_component_register(
    SRCS ${DALHAL_SRCS}
    INCLUDE_DIRS
        "${REPO_ROOT}/src"
        ${PORT_DIRS}
        "${LIBDEPS}/ArduinoJson/src"
        "${LIBDEPS}/Time"
        "${LIBDEPS}/TimeAlarms"
        "${REPO_ROOT}/lib/PubSubClient/src"
    REQUIRES driver esp_timer esp_system freertos lwip esp_wifi esp_netif esp_event nvs_flash esp_http_server app_update esp_partition spi_flash
)

# ESP32 = chip family, as the Arduino core defines it. DALHAL_BACKEND_ESPIDF comes from Platform.h
# (ESP_PLATFORM without ARDUINO) and tells the Arduino core and the ESP-IDF backend apart.
# board define, selects the GPIO list in DALHAL_GPIO_Manager.cpp (as -D ESP32DEV etc in the Arduino envs)
# override with: idf.py -DDALHAL_BOARD=ESP32WROVER_E_IE build
if(NOT DALHAL_BOARD)
    if(IDF_TARGET STREQUAL "esp32c6")
        set(DALHAL_BOARD waveshare_esp32c6_zero)
    elseif(IDF_TARGET STREQUAL "esp32c3")
        set(DALHAL_BOARD seeed_xiao_esp32c3)
    else()
        set(DALHAL_BOARD ESP32DEV)
    endif()
endif()
target_compile_definitions(${COMPONENT_LIB} PUBLIC ESP32 ${DALHAL_BOARD})
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-unused-variable -Wno-unused-function)

# FrontendFiles/Generated from FrontendFiles/www (the same generator as the PlatformIO and PC builds),
# only rewrites the header when www/ changed
if(NOT CMAKE_BUILD_EARLY_EXPANSION)
    idf_build_get_property(python PYTHON)
    execute_process(COMMAND ${python} "${REPO_ROOT}/tools/gen_frontend.py" WORKING_DIRECTORY "${REPO_ROOT}")
endif()
