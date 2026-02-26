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

#include <Arduino.h> // Needed for String class
#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
#include <iostream> // including this take 209512 bytes flash
#include <chrono>
#endif
#include <string>
#include <cstdint>

#include <vector>
#include <stack>


#include "../../Core/Types/DALHAL_ZeroCopyString.h"

namespace DALHAL {
    namespace ScriptEngine {
        struct Token : public ZeroCopyString {
            using ZeroCopyString::ZeroCopyString;

            uint16_t line;
            uint16_t column;

            void Set(const char* _start, const char* _end, int line, int column);
            void ReportTokenInfo(const char* msg, const char* param = nullptr) const;
            void ReportTokenError(const char* msg, const char* param = nullptr) const;
            void ReportTokenWarning(const char* msg, const char* param = nullptr) const;

            Token();
            ~Token();
        };
        /** Tokens are considered identical if their 'start' pointers are the same */
        inline bool operator==(const Token& lhs, const Token& rhs) { return lhs.start == rhs.start; }
        /** Tokens are considered not identical if their 'start' pointers are not the same */
        inline bool operator!=(const Token& lhs, const Token& rhs) { return lhs.start != rhs.start; }
    }
}