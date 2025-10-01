/*
  Dalhalla IoT — JSON-configured HAL/DAL + Script Engine
  HAL = Hardware Abstraction Layer
  DAL = Device Abstraction Layer

  Provides IoT firmware building blocks for home automation and smart sensors.

  Copyright (C) 2025 Jannik Svensson

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or 
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License 
  along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

//#define DEBUG_PRINT_SCRIPT_ENGINE

#include <Arduino.h> // Needed for String class
#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
#include <iostream> // including this take 209512 bytes flash

#endif
#include <string>
#include <cstdint>


#include <vector>
#include <stack>
#include "../../../Support/ConvertHelper.h"
#include "../../../Support/CharArrayHelpers.h"

#include "../../HAL_JSON_ZeroCopyString.h"
#include "../HAL_JSON_SCRIPT_ENGINE_Token.h"
#include "../HAL_JSON_SCRIPT_ENGINE_Script_Token.h"
#include "../HAL_JSON_SCRIPT_ENGINE_Expression_Token.h"
#include "../HAL_JSON_SCRIPT_ENGINE_Expression_Parser.h"




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



namespace HAL_JSON {
    namespace ScriptEngine {
        namespace Parser {

            bool ValidateParseScript(ScriptTokens& tokens, bool validateOnly);
            /** 
             * if the callback is set this is considered a Load function
             * if the callback is not set (nullptr) then it's validate only
             */
            bool ReadAndParseScriptFile(const char* filePath, void (*parsedOKcallback)(ScriptTokens& tokens) = nullptr);
            
        }
    }
}