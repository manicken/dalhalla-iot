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
#include <vector>
#include <cstring>
#include <math.h>

#include <DALHAL/Core/Types/DALHAL_Registry.h>

namespace DALHAL {

    namespace JsonSchema {

        namespace ToJsonString {

            struct JsonKeyValue {
                const char* key; 
                std::string value;
            };

            bool registerContains(const char* key);

            bool inlinesContains(const char* key);
            void addToInlines(const char* key, std::string contents);

            void addRegistrySchemaAndBuild(const Registry::Item* reg, const char* regPath);
            void buildCompleteJsonSchemasStartingFrom(const Registry::Item* reg, std::string &out);

            void clear();

            void appendQuoted(std::string& out, const char* str);
            void appendKey(std::string& out, const char* key);
            void appendBool(std::string& out, bool v);
            void appendBool(std::string& out, const char* key, bool v);
            void appendNumber(std::string& out, const char* key, unsigned int v);
            void appendNumber(std::string& out, const char* key, int v);
            void appendNumber(std::string& out, const char* key, float v);
            void appendString(std::string& out, const char* key, const char* cStr);

        }

    }
}