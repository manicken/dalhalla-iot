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
#include <ArduinoJSON.h>

#include <DALHAL/Support/DALHAL_Logger.h>

#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_TypeBase.h>
#include <DALHAL/Core/Types/DALHAL_Value.h>
#include <DALHAL/Core/Types/DALHAL_Registry.h>

namespace DALHAL {

    namespace JsonSchema {
        

        // used to validate the schema itself
        using SchemaValidatorFn = void (*)(const SchemaTypeBase&, const char* sourceObjTypeName, bool& anyError);
        using JsonValidatorFn = ValidatorResult (*)(const SchemaTypeBase&, const char* sourceObjTypeName, const JsonVariant&, bool& anyError);
        using JsonValueExtractFn = HALValue (*)(const SchemaTypeBase&, const JsonVariant&);
        using SchemaToJsonFn = void (*)(const SchemaTypeBase&, std::string& jsonStr);
        using GetJavaScriptValidatorFn = const char* (*)();

        struct FieldTypeRegistryDefine {
            SchemaValidatorFn ValidateSchema = nullptr;
            JsonValidatorFn ValidateJson = nullptr;
            JsonValueExtractFn GetValue = nullptr;
            SchemaToJsonFn ToJson = nullptr;
            GetJavaScriptValidatorFn GetJavaScriptValidator = nullptr;

            constexpr FieldTypeRegistryDefine(
                SchemaValidatorFn ValidateSchema, 
                JsonValidatorFn ValidateJson,
                SchemaToJsonFn ToJson, 
                GetJavaScriptValidatorFn GetJavaScriptValidator
            ) 
                : 
                ValidateSchema(ValidateSchema), 
                ValidateJson(ValidateJson), 
                ToJson(ToJson), 
                GetJavaScriptValidator(GetJavaScriptValidator)
                {}

            constexpr FieldTypeRegistryDefine(
                SchemaValidatorFn ValidateSchema, 
                JsonValidatorFn ValidateJson,
                JsonValueExtractFn GetValue,
                SchemaToJsonFn ToJson, 
                GetJavaScriptValidatorFn GetJavaScriptValidator
            ) 
                : 
                ValidateSchema(ValidateSchema), 
                ValidateJson(ValidateJson),
                GetValue(GetValue),
                ToJson(ToJson), 
                GetJavaScriptValidator(GetJavaScriptValidator)
                {}
            
        };

        struct FieldTypeRegistryItem {
            const char* name;
            FieldTypeRegistryDefine define;
        };

        extern const FieldTypeRegistryItem g_fieldTypeTable[
            static_cast<size_t>(FieldType::_Count_)
        ];

        const FieldTypeRegistryItem& GetFieldTypeRegistryItem(FieldType type);

        const char* FieldTypeToString(FieldType type);

        void ValidateSchema(const SchemaTypeBase& stb, const char* sourceObjTypeName, bool& anyError);
        ValidatorResult ValidateJson(const SchemaTypeBase& stb, const char* sourceObjTypeName, const JsonVariant& jsonObj, bool& anyError);
        void SchemaToJson(const SchemaTypeBase& stb, std::string& jsonStr);

        HALValue GetValue(const SchemaTypeBase& stb, const JsonVariant& jsonObj);
        HALValue GetValue(const SchemaTypeBase& stb, const DeviceCreateContext& context);
        
        const char* GetJavaScriptValidator(const SchemaTypeBase& stb);
        

    }

}