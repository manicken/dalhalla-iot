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

#include "DALHAL_JSON_Schema_String.h"

#include <stdlib.h>
#include <ArduinoJson.h>

#include <DALHAL/Support/DALHAL_Logger.h>

#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_TypeBase.h>
#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_ValidatorResult.h>

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_TypesRegistry.h>

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_ToJsonStringHelpers.h>

namespace DALHAL {

    namespace JsonSchema {

        __attribute__((used, externally_visible))
        constexpr FieldTypeRegistryDefine SchemaString::RegistryDefine = {
            &ValidateSchema,
            &ValidateJson,
            &GetValue,
            &SchemaToJson,
            &GetJavaScriptValidator
        };
        //volatile const void* keep_SchemaString = &DALHAL::JsonSchema::SchemaString::RegistryDefine;

        void SchemaString::ValidateSchema(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, bool& anyError) {
            if (SchemaTypeBase::ValidateSchemaNameNotNull(fieldSchema, sourceObjTypeName) == false) {
                anyError = true;
            }
        }

        ValidatorResult SchemaString::ValidateStringField(const char* fieldName, const char* sourceObjTypeName, const JsonVariant& jsonObj, bool& anyError) {
            const JsonVariant& value = jsonObj[fieldName];
            if (!value.is<const char*>()) {
                std::string errMsg = fieldName;
                errMsg += " @ "; errMsg += sourceObjTypeName;
                GlobalLogger.Error(F("Field must be a string:"), errMsg.c_str());
                anyError = true;
                return ValidatorResult::FieldTypeMismatch;
            }
            ZeroCopyString zcStr = value.as<const char*>(); // wrap in ZeroCopyString for neat functions
            zcStr.Trim();
            size_t strLen = zcStr.Length(); // use of lenght here is fast
            
            if (strLen == 0) {
                std::string errMsg = fieldName;
                errMsg += " @ "; errMsg += sourceObjTypeName;
                GlobalLogger.Error(F("String is empty: "), errMsg.c_str());
                anyError = true;
                return ValidatorResult::FieldEmpty;
            }
            return ValidatorResult::Success;
        }

        ValidatorResult SchemaString::ValidateJson(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, const JsonVariant& jsonObj, bool& anyError) {
            ValidatorResult vRes = SchemaTypeBase::ValidateFieldPresenceAndPolicy(fieldSchema, sourceObjTypeName, jsonObj, anyError);
            if (vRes != ValidatorResult::Success) {
                return vRes; 
            }
            return SchemaString::ValidateStringField(fieldSchema.name, sourceObjTypeName, jsonObj, anyError);
            /*const JsonVariant& value = jsonObj[fieldSchema.name];
            if (!value.is<const char*>()) {
                GlobalLogger.Error(F("Field must be a string:"), fieldSchema.name);
                anyError = true;
                return ValidatorResult::FieldTypeMismatch;
            }
            ZeroCopyString zcStr = value.as<const char*>(); // wrap in ZeroCopyString for neat functions
            zcStr.Trim();
            size_t strLen = zcStr.Length(); // use of lenght here is fast
            
            if (strLen == 0) {
                GlobalLogger.Error(F("String is empty: "), fieldSchema.name);
                anyError = true;
                return ValidatorResult::FieldEmpty;
            }
            return ValidatorResult::Success;*/
        }

        const char* SchemaString::ExtractFrom(const JsonVariant& jsonObj) const {
            if (jsonObj.containsKey(this->name)) {
                return jsonObj[this->name].as<const char*>();
            } else {
                return this->defaultValue;
            }
        }

        HALValue SchemaString::GetValue(const SchemaTypeBase& fieldSchema, const JsonVariant& jsonObj) {
            return HALValue(static_cast<const SchemaString&>(fieldSchema).ExtractFrom(jsonObj));
        }

        void SchemaString::SchemaToJson(const SchemaTypeBase& fieldSchema, std::string& out) {
            SchemaTypeBase::SchemaToJson(fieldSchema, out);
            const SchemaString& strSchema = static_cast<const SchemaString&>(fieldSchema);
            if (strSchema.defaultValue != nullptr) {
                out += ','; ToJsonString::appendString(out, "default", strSchema.defaultValue);
            }
            if (fieldSchema.type == FieldType::String) {
                out += '}'; // add the object finalizer if this is the actual object
            }
        }

        const char* SchemaString::GetJavaScriptValidator() { return R"rawliteral(
            function validateString(value) {
                if (value == undefined) {
                    // emit not a string error here
                    return false;
                }
                if (value.length == 0) {
                    // emit string empty error here
                    return false;
                }
                // no other validation is needed on basic strings
                return true;
            }
        )rawliteral";

        }
    }

}