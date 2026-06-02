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

#include <DALHAL/API/DALHAL_CommandCallback.h>

namespace DALHAL {

    namespace JsonSchema {

        namespace ToJsonString {

            struct DeviceRegistryQueueItem {
                const char* regPath;
                const Registry::DeviceRegistry& reg;
            };

            struct JsonObjectSchemaQueueItem {
                const char* id; 
                const JsonObjectSchema& schema;
            };

            struct InlineQueueItem {
                const char* id;
                const SchemaTypeBase& schema;
            };

            bool registerContains(const char* id);
            bool objectsContains(const char* id);
            bool inlinesContains(const char* id);

            void addToRegistries(const char* regPath, const Registry::DeviceRegistry& reg);
            void addToObjects(const char* id, const JsonObjectSchema& schema);
            void addToInlines(const char* id, const SchemaTypeBase& schema);

            void buildCompleteJsonSchemasStartingFrom(const Registry::DeviceRegistry& reg, CommandCallback cb);

            void clear();


        }

    }
}