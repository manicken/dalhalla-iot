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

#include "DALHAL_JSON_Schema_StringBase.h"

#include <stdlib.h>
#include <ArduinoJson.h>

#include <DALHAL/Support/DALHAL_Logger.h>

#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_TypeBase.h>

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_TypesRegistry.h>

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_ToJsonStringHelpers.h>

namespace DALHAL {

    namespace JsonSchema {

        

        constexpr FieldTypeRegistryDefine SchemaStringBase::RegistryDefine = {
            &SchemaValidate,
            &ValidateJson,
            &SchemaToJson,
            JavaScriptValidator
        };

        void SchemaStringBase::SchemaValidate(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, bool& anyError) {
            if (SchemaTypeBase::SchemaValidateNameNotNull(fieldSchema, sourceObjTypeName) == false) {
                anyError = true;
            }
        }

        ValidatorResult SchemaStringBase::ValidateJson(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, const JsonVariant& jsonObj, bool& anyError) {
            ValidatorResult res = SchemaTypeBase::ValidateJson(fieldSchema, sourceObjTypeName, jsonObj, anyError);
            if (res != ValidatorResult::Success) {
                return res; // base do currently only return ValidatorResult::RequiredFieldMissing
            }
            const JsonVariant& value = jsonObj[fieldSchema.name];
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
            return ValidatorResult::Success;
        }

        void SchemaStringBase::SchemaToJson(const SchemaTypeBase& fieldSchema, std::string& out) {
            SchemaTypeBase::SchemaToJson(fieldSchema, out);
            const SchemaStringBase& strSchema = static_cast<const SchemaStringBase&>(fieldSchema);
            if (strSchema.defaultValue != nullptr) {
                ToJsonString::appendString(out, "default", strSchema.defaultValue);
            }
            if (fieldSchema.type == FieldType::StringBase) {
                out += '}'; // add the object finalizer if this is the actual object
            }
        }

        const char* SchemaStringBase::JavaScriptValidator = R"rawliteral(
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