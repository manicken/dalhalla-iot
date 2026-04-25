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

#include "DALHAL_JSON_Schema_UInt.h"

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

        constexpr FieldTypeRegistryDefine SchemaUInt::RegistryDefine = {
            &ValidateSchema,
            &ValidateJson,
            &GetValue,
            &SchemaToJson,
            &GetJavaScriptValidator
        };

        void SchemaUInt::ValidateSchema(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, bool& anyError) {
            if (SchemaTypeBase::ValidateSchemaNameNotNull(fieldSchema, sourceObjTypeName) == false) {
                anyError = true;
            }
            const SchemaUInt& uintSchema = static_cast<const SchemaUInt&>(fieldSchema);
            if (uintSchema.maxValue < uintSchema.minValue) {
                GlobalLogger.Error(F("schema error - uintSchema.maxValue < uintSchema.minValue @ "), sourceObjTypeName);
                anyError = true;
            }
        }

        ValidatorResult SchemaUInt::ValidateJson(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, const JsonVariant& jsonObj, bool& anyError) {
            ValidatorResult vRes = SchemaTypeBase::ValidateFieldPresenceAndPolicy(fieldSchema, sourceObjTypeName, jsonObj, anyError);
            if (vRes != ValidatorResult::Success) {
                return vRes; 
            }

            const JsonVariant& value = jsonObj[fieldSchema.name];
            if (!value.is<unsigned int>()) {
                std::string errStr = fieldSchema.name; errStr += " @ ";
                serializeCollapsed(jsonObj, errStr);
                GlobalLogger.Error(F(" must be unsigned int: "), errStr.c_str());

                anyError = true;
                return ValidatorResult::FieldTypeMismatch;
            }

            auto fs = static_cast<const SchemaUInt&>(fieldSchema);
            unsigned int v = value.as<unsigned int>();
            if (((fs.minValue != 0) && (v < fs.minValue)) || ((fs.maxValue != 0) && (v > fs.maxValue))) { // if maxValue == 0 then the value can be anything
                std::string errStr = fieldSchema.name; errStr += " @ ";
                serializeCollapsed(jsonObj, errStr);
                GlobalLogger.Error(F(" uint out of range: "), errStr.c_str());

                anyError = true;
                return ValidatorResult::FieldInvalidValue;
            }
            return ValidatorResult::Success;
        }

        unsigned int SchemaUInt::ExtractFrom(const JsonVariant& jsonObj) const {
            unsigned int raw;
            if (jsonObj.containsKey(this->name)) {
                raw = jsonObj[this->name].as<unsigned int>();
            } else {
                raw = this->defaultValue;
            }
            if (this->HasConversion() == false) {
                return raw;
            }
            return static_cast<unsigned int>(raw * this->conversionFactor);
        }

        HALValue SchemaUInt::GetValue(const SchemaTypeBase& fieldSchema, const JsonVariant& jsonObj) {
            return HALValue(static_cast<const SchemaUInt&>(fieldSchema).ExtractFrom(jsonObj));
        }

        void SchemaUInt::SchemaToJson(const SchemaTypeBase& fieldSchema, std::string& out) {
            SchemaTypeBase::SchemaToJson(fieldSchema, out);
            const SchemaUInt& fs = static_cast<const SchemaUInt&>(fieldSchema);
            out += ','; ToJsonString::appendNumber(out, "default", fs.defaultValue);
            out += ','; ToJsonString::appendNumber(out, "minValue", fs.minValue);
            out += ','; ToJsonString::appendNumber(out, "maxValue", fs.maxValue);
            
            if (fieldSchema.type == FieldType::UInt) {
                out += '}'; // add the object finalizer if this is the actual object
            }
        }

        const char* SchemaUInt::GetJavaScriptValidator() {
            return R"rawliteral(

            )rawliteral";
        }

    }

}