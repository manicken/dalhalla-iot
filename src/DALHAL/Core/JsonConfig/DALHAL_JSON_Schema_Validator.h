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

#include "DALHAL_JSON_Schema_Types.h"

#include <ArduinoJSON.h>
using json = JsonVariant;

namespace DALHAL {

    namespace JsonSchema {

        bool isUnknownField(const char* key, const FieldBase* const* fields);
        // Helper to validate FieldString / FieldUID
        void validateStringField(const JsonVariant& value, const FieldString* f, bool& anyError);
        // Validate a single field
        void validateField(const JsonVariant& j, const FieldBase* field, bool& anyError);
        // Validate OneOfGroup
        void validateOneOfGroup(const JsonVariant& j, const char* fieldName, const OneOfGroup* group, bool& anyError);
        // Validate AllOfGroup
        void validateAllOfGroup(const JsonVariant& j, const char* fieldName, const AllOfGroup* group, bool& anyError);
        // Validate ModeSelector
        int evaluateModes(const JsonVariant& j, const ModeSelector* modes);
        // Validate a JsonArray
        void validateJsonArray(const JsonVariant& j, const FieldArray* field, bool& anyError);
        // Validate a simple JsonArray of primitives such as bool,uint,int,float
        void validateJsonArrayPrimitive(const JsonVariant& j, const JsonSchema::FieldArrayPrimitive* field, bool& anyError);
        // Validate a full device
        void validateJsonObject(const JsonVariant& j, const char* fieldName, const JsonSchema::JsonObjectSchema* jsonObjectSchema, bool& anyError);
        // Validate the JSON array against the given device registry.
        void validateFromRegister(const JsonVariant& jsonArray, const Registry::Item* reg, bool& anyError);

    }

}