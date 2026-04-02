cmake_minimum_required(VERSION 3.10)
project(DALHAL_WebSocket_Windows)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add source files
add_executable(WebSocket_Server
    WebSocket_Example_Windows.cpp
    DALHAL_WebSocketAPI_Windows.cpp
)

# Include directories
target_include_directories(WebSocket_Server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Link Windows socket libraries
if(WIN32)
    target_link_libraries(WebSocket_Server PRIVATE
        ws2_32
        mswsock
        advapi32
    )
endif()

# Compiler flags for different configurations
if(MSVC)
    target_compile_options(WebSocket_Server PRIVATE /W4 /WX)
    set_target_properties(WebSocket_Server PROPERTIES
        VS_DEBUGGER_COMMAND "$(TargetPath)"
    )
else()
    target_compile_options(WebSocket_Server PRIVATE -Wall -Wextra -Wpedantic)
endif()
