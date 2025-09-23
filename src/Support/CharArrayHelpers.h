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


#include <Arduino.h>

#include <cctype>
#include <string>
#include <cstring>
#include <cstdint>

    
namespace CharArray {

    uint32_t countChar(const char* str, char ch);
    const uint32_t* getIndicies(const char* str, char ch, uint32_t& outCount);
    
    const char** getPointers(const char* str, char ch, uint32_t& outCount);
    /** case insensitive compare */
    bool equalsIgnoreCase(const char* a, const char* b);

    int count_tokens(const char* str, const char* delimiters);
    char *my_strtok(char **str, const char *delimiters);

    bool EndsWith(const char* str, const char* suffix);
    /*
    struct StringView {
        const char* ptr;
        size_t len;

        StringView();
        StringView(const char* p, size_t l);

        bool empty() const;

        std::string toString() const;
    };*/
    bool StrEqualsICAny(const char* text, const char* const* candidates);
}

inline bool StrEqualsIC(const char* strA, const char* strB) { return CharArray::equalsIgnoreCase(strA, strB); }
