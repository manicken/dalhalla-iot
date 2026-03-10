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

#include "DALHAL_JSON_Schema.h"

#include <ArduinoJSON.h>
using json = JsonVariant;

namespace DALHAL {

    namespace JsonSchema {

        bool isKnownField(const char* key, const FieldBase* const* fields);
        // Helper to validate FieldString / FieldUID
        bool validateStringField(const JsonVariant& value, const FieldString* f, std::string& error);
        // Validate a single field
        bool validateField(const JsonObjectConst& j, const FieldBase* field, std::string& error);
        // Validate AnyOfGroup
        bool validateAnyOfGroup(const JsonObjectConst& j, const AnyOfGroup* group, std::string& error);
        // Validate ModeSelector
        int evaluateModes(const JsonObjectConst& j, const ModeSelector* modes);
        // Validate a full device
        int validateDevice(const JsonObjectConst& j, const JsonSchema::Device* device, std::string& error);

    }

}