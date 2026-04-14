/*
  Dalhalla IoT — JSON-configured HAL/DAL + Script Engine
  HAL = Hardware Abstraction Layer
  DAL = Device Abstraction Layer

  Provides IoT firmware building blocks for home automation and smart sensors.

  Copyright (C) 2026 Jannik Svensson

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

#include <string>

namespace DALHAL {

    namespace JsonSchema {

        namespace ToJsonString {

            inline void appendQuoted(std::string& out, const char* str) {
                out += '"';
                if (str) out += str;
                out += '"';
            }

            inline void appendKey(std::string& out, const char* key) {
                appendQuoted(out, key);
                out += ':';
            }

            inline void appendBool(std::string& out, bool v) {
                out += (v ? "true" : "false");
            }

            inline void appendBool(std::string& out, const char* key, bool v) {
                appendKey(out, key);
                appendBool(out, v);
            }

            inline void appendNumber(std::string& out, const char* key, unsigned int v) {
                appendKey(out, key);
                out += std::to_string(v);
            }
            inline void appendNumber(std::string& out, const char* key, int v) {
                appendKey(out, key);
                out += std::to_string(v);
            }
            inline void appendNumber(std::string& out, const char* key, float v) {
                appendKey(out, key);
                out += std::to_string(v);
            }

            inline void appendString(std::string& out, const char* key, const char* cStr) {
                appendKey(out, key);
                appendQuoted(out, cStr);
            }

        }

    }
}