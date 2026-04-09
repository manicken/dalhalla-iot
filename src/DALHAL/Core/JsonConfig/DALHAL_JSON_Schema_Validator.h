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

#include <DALHAL/Core/Types/DALHAL_Registry.h>

#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_JsonObjectSchema.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_StringBase.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_Array.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_ArrayPrimitive.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_ComplexTypes.h>

#include <ArduinoJSON.h>
using json = JsonVariant;

namespace DALHAL {

    namespace JsonSchema {

        bool isUnknownField(const char* key, const SchemaTypeBase* const* fields);
        // Helper to validate FieldString / FieldUID
        void validateStringField(const JsonVariant& value, const char* sourceObjTypeName, const SchemaStringBase* f, bool& anyError);
        // Validate a single field
        void validateField(const JsonVariant& j, const char* sourceObjTypeName, const SchemaTypeBase* field, bool& anyError);
        // Validate OneOfGroup
        void validateOneOfGroup(const JsonVariant& j, const char* fieldName, const SchemaOneOfFieldsGroup* group, bool& anyError);
        // Validate AllOfGroup
        void validateAllOfGroup(const JsonVariant& j, const char* fieldName, const SchemaAllOfFieldsGroup* group, bool& anyError);
        // Validate ModeSelector
        int evaluateModes(const JsonVariant& j, const ModeSelector* modes);
        // Validate a JsonArray
        void validateJsonArray(const JsonVariant& j, const SchemaArray* field, bool& anyError);
        // Validate a simple JsonArray of primitives such as bool,uint,int,float
        void validateJsonArrayPrimitive(const JsonVariant& j, const SchemaArrayPrimitive* field, bool& anyError);
        // Validate a full device
        void validateJsonObject(const JsonVariant& j, const char* fieldName, const JsonObjectSchema* jsonObjectSchema, bool& anyError);
        // Validate the JSON array against the given device registry.
        void validateFromRegister(const JsonVariant& jsonArray, const Registry::Item* reg, bool& anyError);

    }

}