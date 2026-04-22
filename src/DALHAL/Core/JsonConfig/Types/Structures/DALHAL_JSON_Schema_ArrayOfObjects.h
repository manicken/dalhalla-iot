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

#include <stdlib.h>

#include <DALHAL/Support/DALHAL_Logger.h>

#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_TypeBase.h>
#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_ValidatorResult.h>
#include <DALHAL/Core/JsonConfig/Types/Root/DALHAL_JSON_Schema_JsonObjectSchema.h>
#include <DALHAL/Core/JsonConfig/Types/Root/DALHAL_JSON_Schema_EmptyPolicy.h>
#include <DALHAL/Core/JsonConfig/Types/Structures/DALHAL_JSON_Schema_ArrayBase.h>

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_TypesRegistry.h>

namespace DALHAL {

    namespace JsonSchema {

        /** 
         * FieldArray represents a homogeneous array where every element must conform 
         * to the same JsonSchema::Device definition. This is used for structured data 
         * with a fixed schema (no type selection per element).
         */
        struct SchemaArrayOfObjects : SchemaArrayBase {
            
            static const FieldTypeRegistryDefine RegistryDefine;
            static void ValidateSchema(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, bool& anyError);
            static ValidatorResult ValidateJson(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, const JsonVariant& jsonObj, bool& anyError);
            static const JsonArray& GetValidatedJsonArray(const SchemaArrayOfObjects& saoo, const JsonVariant& jsonObj);
            static void SchemaToJson(const SchemaTypeBase& fieldSchema, std::string& out);
            static const char* GetJavaScriptValidator();

            const JsonObjectSchema* subtype;
            const SchemaTypeBase* renderAllAllowedValuesFromStringConstraint;

            constexpr SchemaArrayOfObjects(const char* name, FieldPolicy policy, const JsonObjectSchema* subtype)
                : SchemaArrayBase(name, FieldType::ArrayOfObjects, policy, EmptyPolicy::Warn), subtype(subtype), renderAllAllowedValuesFromStringConstraint(nullptr) {}
            constexpr SchemaArrayOfObjects(const char* name, FieldPolicy policy, const JsonObjectSchema* subtype, EmptyPolicy emptyPolicy)
                : SchemaArrayBase(name, FieldType::ArrayOfObjects, policy, emptyPolicy), subtype(subtype), renderAllAllowedValuesFromStringConstraint(nullptr) {}

            constexpr SchemaArrayOfObjects(const char* name, FieldPolicy policy, FieldGuiFlagsType guiFlags, const JsonObjectSchema* subtype)
                : SchemaArrayBase(name, FieldType::ArrayOfObjects, policy, guiFlags, EmptyPolicy::Warn), subtype(subtype), renderAllAllowedValuesFromStringConstraint(nullptr) {}
            constexpr SchemaArrayOfObjects(const char* name, FieldPolicy policy, FieldGuiFlagsType guiFlags, const JsonObjectSchema* subtype, EmptyPolicy emptyPolicy)
                : SchemaArrayBase(name, FieldType::ArrayOfObjects, policy, guiFlags, emptyPolicy), subtype(subtype), renderAllAllowedValuesFromStringConstraint(nullptr) {}

            constexpr SchemaArrayOfObjects(const char* name, FieldPolicy policy, FieldGuiFlagsType guiFlags, const SchemaTypeBase* renderAllAllowedValuesFromStringConstraint, const JsonObjectSchema* subtype)
                : SchemaArrayBase(name, FieldType::ArrayOfObjects, policy, guiFlags, EmptyPolicy::Warn), subtype(subtype), renderAllAllowedValuesFromStringConstraint(renderAllAllowedValuesFromStringConstraint) {}
            constexpr SchemaArrayOfObjects(const char* name, FieldPolicy policy, FieldGuiFlagsType guiFlags, const SchemaTypeBase* renderAllAllowedValuesFromStringConstraint, const JsonObjectSchema* subtype, EmptyPolicy emptyPolicy)
                : SchemaArrayBase(name, FieldType::ArrayOfObjects, policy, guiFlags, emptyPolicy), subtype(subtype), renderAllAllowedValuesFromStringConstraint(renderAllAllowedValuesFromStringConstraint) {}
        };

    }

}