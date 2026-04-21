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

#include "DALHAL_JSON_Schema_StringAnyOfByFuncConstrained.h"

#include <stdlib.h>
#include <ArduinoJson.h>

#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_TypeBase.h>
#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_ValidatorResult.h>
#include <DALHAL/Core/JsonConfig/Types/Primitives/DALHAL_JSON_Schema_String.h>

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_TypesRegistry.h>

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_ToJsonStringHelpers.h>

namespace DALHAL {

    namespace JsonSchema {

        constexpr FieldTypeRegistryDefine SchemaStringAnyOfByFuncConstrained::RegistryDefine = {
            &ValidateSchema,
            &ValidateJson,
            &SchemaString::GetValue,
            &SchemaToJson,
            &GetJavaScriptValidator
        };

        void SchemaStringAnyOfByFuncConstrained::ValidateSchema(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, bool& anyError) {
            SchemaString::ValidateSchema(fieldSchema, sourceObjTypeName, anyError);
            const SchemaStringAnyOfByFuncConstrained& strSchema = static_cast<const SchemaStringAnyOfByFuncConstrained&>(fieldSchema);
            if (strSchema.describe == nullptr) {
                GlobalLogger.Error(F("SchemaStringAnyOfByFuncConstrained schema error - strSchema.describe == nullptr @ "), sourceObjTypeName);
                anyError = true;
            }
            if (strSchema.validate == nullptr) {
                GlobalLogger.Error(F("SchemaStringAnyOfByFuncConstrained schema error - strSchema.validate == nullptr @ "), sourceObjTypeName);
                anyError = true;
            }
            if (strSchema.ctx == nullptr) {
                GlobalLogger.Error(F("SchemaStringAnyOfByFuncConstrained schema error - strSchema.ctx == nullptr @ "), sourceObjTypeName);
                anyError = true;
            }
        }
        
        ValidatorResult SchemaStringAnyOfByFuncConstrained::ValidateJson(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, const JsonVariant& jsonObj, bool& anyError) {
            ValidatorResult res = SchemaString::ValidateJson(fieldSchema, sourceObjTypeName, jsonObj, anyError);

            if (res != ValidatorResult::Success) {
                return res; //  this mean either this field dont exist or that it's not a valid string 
                // so it can't continue 
            }
            const char* cStr = jsonObj[fieldSchema.name].as<const char*>(); // this is now safe
            const SchemaStringAnyOfByFuncConstrained& strSchema = static_cast<const SchemaStringAnyOfByFuncConstrained&>(fieldSchema);
            if (strSchema.validate(strSchema.ctx, cStr) == false) {
                anyError = true;
                std::string errMsg;
                errMsg += fieldSchema.name;
                errMsg += " @ "; errMsg += '['; errMsg += sourceObjTypeName; errMsg += ']';
                GlobalLogger.Error(F("Invalid value for field: "), errMsg.c_str());
                return ValidatorResult::FieldInvalidValue;
            }
            return ValidatorResult::Success;
        }

        void SchemaStringAnyOfByFuncConstrained::SchemaToJson(const SchemaTypeBase& fieldSchema, std::string& out) {
            SchemaString::SchemaToJson(fieldSchema, out);
            out += ','; ToJsonString::appendKey(out, "allowedValues");
            const SchemaStringAnyOfByFuncConstrained& strSchema = static_cast<const SchemaStringAnyOfByFuncConstrained&>(fieldSchema);
            out += strSchema.describe(strSchema.ctx);
            
            if (fieldSchema.type == FieldType::StringAnyOfByFuncConstrained) {
                out += '}'; // this is complete object
            }
        }

        const char* SchemaStringAnyOfByFuncConstrained::GetJavaScriptValidator() {
            return R"rawliteral(
            function validateString(value) {

            }
            )rawliteral";
        }
    }

}