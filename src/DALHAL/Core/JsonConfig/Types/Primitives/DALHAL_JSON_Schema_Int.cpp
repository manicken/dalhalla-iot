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

#include "DALHAL_JSON_Schema_Int.h"

#include <stdlib.h>

#include <ArduinoJson.h>
#include <DALHAL/Core/JsonConfig/DALHAL_ArduinoJSON_ext.h>

#include <DALHAL/Support/DALHAL_Logger.h>

#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_TypeBase.h>
#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_ValidatorResult.h>

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_TypesRegistry.h>

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_ToJsonStringHelpers.h>

namespace DALHAL {

    namespace JsonSchema {

        __attribute__((used, externally_visible))
        constexpr FieldTypeRegistryDefine SchemaInt::RegistryDefine = {
            &ValidateSchema,
            &ValidateJson,
            &GetValue,
            &SchemaToJson,
            &GetJavaScriptValidator
        };
        //volatile const void* keep_SchemaInt = &DALHAL::JsonSchema::SchemaInt::RegistryDefine;

        void SchemaInt::ValidateSchema(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, bool& anyError) {
            if (SchemaTypeBase::ValidateSchemaNameNotNull(fieldSchema, sourceObjTypeName) == false) {
                anyError = true;
            }
            const SchemaInt& intSchema = static_cast<const SchemaInt&>(fieldSchema);
            if (intSchema.maxValue < intSchema.minValue) {
                GlobalLogger.Error(F("schema error - intSchema.maxValue < intSchema.minValue @ "), sourceObjTypeName);
                anyError = true;
            }
        }

        ValidatorResult SchemaInt::ValidateJson(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, const JsonVariant& jsonObj, bool& anyError) {
            ValidatorResult vRes = SchemaTypeBase::ValidateFieldPresenceAndPolicy(fieldSchema, sourceObjTypeName, jsonObj, anyError);
            if (vRes != ValidatorResult::Success) {
                return vRes; 
            }

            const JsonVariant& value = jsonObj[fieldSchema.name];
            if (!value.is<int>()) {
                std::string errStr = fieldSchema.name; errStr += " @ ";
                serializeCollapsed(jsonObj, errStr);
                GlobalLogger.Error(F(" must be signed int: "), errStr.c_str());

                anyError = true;
                return ValidatorResult::FieldTypeMismatch;
            }

            auto fs = static_cast<const SchemaInt&>(fieldSchema);
            int v = value.as<int>();
            if (((fs.minValue != 0) && (v < fs.minValue)) || ((fs.maxValue != 0) && (v > fs.maxValue))) { // if maxValue == 0 then the value can be anything
                std::string errStr = fieldSchema.name; errStr += " @ ";
                serializeCollapsed(jsonObj, errStr);
                GlobalLogger.Error(F(" signed int out of range: "), errStr.c_str());

                anyError = true;
                return ValidatorResult::FieldInvalidValue;
            }
            return ValidatorResult::Success;
        }

        int SchemaInt::ExtractFrom(const JsonVariant& jsonObj) const {
            int raw;
            if (jsonObj.containsKey(this->name)) {
                raw = jsonObj[this->name].as<signed int>();
            } else {
                raw = this->defaultValue;
            }
            if (this->HasConversion() == false) {
                return raw;
            }
            return static_cast<int>(raw * this->conversionFactor);
        }

        HALValue SchemaInt::GetValue(const SchemaTypeBase& fieldSchema, const JsonVariant& jsonObj) {
            return HALValue(static_cast<const SchemaInt&>(fieldSchema).ExtractFrom(jsonObj));
        }

        void SchemaInt::SchemaToJson(const SchemaTypeBase& fieldSchema, std::string& out) {
            SchemaTypeBase::SchemaToJson(fieldSchema, out);
            auto fs = static_cast<const SchemaInt&>(fieldSchema);
            out += ','; ToJsonString::appendNumber(out, "default", fs.defaultValue);
            out += ','; ToJsonString::appendNumber(out, "minValue", fs.minValue);
            out += ','; ToJsonString::appendNumber(out, "maxValue", fs.maxValue);
            
            if (fieldSchema.type == FieldType::Int) {
                out += '}'; // add the object finalizer if this is the actual object
            }
        }

        const char* SchemaInt::GetJavaScriptValidator() {
            return R"rawliteral(

            )rawliteral";
        }

    }

}