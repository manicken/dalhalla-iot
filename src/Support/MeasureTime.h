#pragma once

#define HAL_JSON_SCRIPT_ENGINE_PARSER_DEBUG_TIMES

#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
#include <chrono>
#include <iostream>

#define MEASURE_TIME(message, block) \
do { \
    auto _mt_start = std::chrono::high_resolution_clock::now(); \
    block; \
    auto _mt_end = std::chrono::high_resolution_clock::now(); \
    std::chrono::duration<double, std::milli> _mt_duration = _mt_end - _mt_start; \
    std::cout << message << _mt_duration.count() << " ms\n"; \
} while (0)
#elif defined(HAL_JSON_SCRIPT_ENGINE_PARSER_DEBUG_TIMES)
#include <Arduino.h>
#define MEASURE_TIME(message, block) \
do { \
    auto _mt_start = micros(); \
    block; \
    auto _mt_end = micros(); \
    auto _mt_duration = _mt_end - _mt_start; \
    printf("\n%s %d us\n",message,_mt_duration); \
} while (0)
#else
// On embedded builds: expands to nothing, zero overhead
#define MEASURE_TIME(message, block) do { block; } while (0)
#endif