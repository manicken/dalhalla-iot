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

namespace DALHAL {

    namespace JsonSchema {
        enum class ValidatorResult {
            Success,
            FieldTypeMismatch,
            FieldEmpty,
            RequiredFieldMissing,
            FieldInvalidValue
        };
        const char* ValidatorResultToString(ValidatorResult res);

        // used to validate the schema itself
        using SchemaValidatorFn = void (*)(const SchemaTypeBase&, const char* sourceObjTypeName, bool& anyError);
        using ValidatorFn = ValidatorResult (*)(const SchemaTypeBase&, const char* sourceObjTypeName, const JsonVariant&, bool& anyError);
        using SchemaToJsonFn = void (*)(const SchemaTypeBase&, std::string& jsonStr);

        struct FieldTypeRegistryDefine {
            SchemaValidatorFn schemaValidator = nullptr;
            ValidatorFn validator = nullptr;
            SchemaToJsonFn toJson = nullptr;
            const char* validateJavascript = nullptr;

            constexpr FieldTypeRegistryDefine(SchemaValidatorFn schemaValidator, ValidatorFn validator, SchemaToJsonFn toJson, const char* validateJavascript) 
                : schemaValidator(schemaValidator), validator(validator), toJson(toJson), validateJavascript(validateJavascript) {}
        };

        struct FieldTypeRegistryItem {
            const char* name;
            FieldTypeRegistryDefine define;
        };

        /*struct FieldTypeDescriptor {
            const char* name;
            ValidatorFn validator;
            SchemaToJsonFn toJson;
            const char* validateJavascript;
        };*/

        extern const FieldTypeRegistryItem g_fieldTypeTable[
            static_cast<size_t>(FieldType::_Count_)
        ];

        const FieldTypeRegistryItem& GetFieldTypeRegistryItem(FieldType type);
        const char* FieldTypeToString(FieldType type);

    }

}