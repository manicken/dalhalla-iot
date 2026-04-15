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

#include "DALHAL_JSON_Schema_StringSizeConstrained.h"

#include <stdlib.h>
#include <ArduinoJson.h>

#include <DALHAL/Support/DALHAL_Logger.h>

#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_TypeBase.h>
#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_ValidatorResult.h>
#include <DALHAL/Core/JsonConfig/Types/Primitives/DALHAL_JSON_Schema_String.h>

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_TypesRegistry.h>

#include "../DALHAL_JSON_Schema_ToJsonStringHelpers.h"


namespace DALHAL {

    namespace JsonSchema {

        constexpr FieldTypeRegistryDefine SchemaStringSizeConstrained::RegistryDefine = {
            &SchemaValidate,
            &ValidateJson,
            &SchemaToJson,
            &GetJavaScriptValidator
        };

        void SchemaStringSizeConstrained::SchemaValidate(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, bool& anyError) {
            SchemaStringBase::SchemaValidate(fieldSchema, sourceObjTypeName, anyError);
            const SchemaStringSizeConstrained& strSchema = static_cast<const SchemaStringSizeConstrained&>(fieldSchema);
            if (strSchema.maxLength < strSchema.minLength) {
                GlobalLogger.Error(F("schema error - strSchema.maxLength < strSchema.minLength @ "), sourceObjTypeName);
                anyError = true;
            }
        }

        ValidatorResult SchemaStringSizeConstrained::ValidateJson(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, const JsonVariant& jsonObj, bool& anyError) {
            ValidatorResult res = SchemaStringBase::ValidateJson(fieldSchema, sourceObjTypeName, jsonObj, anyError);

            if (res != ValidatorResult::Success && res != ValidatorResult::FieldEmpty) {
                return res; //  this mean either this field dont exist or that it's not a valid string 
                // so it can't continue 
            }
            // here a empty string still pass
            const char* cStr = jsonObj[fieldSchema.name].as<const char*>(); // this is now safe
            ZeroCopyString zcStr = cStr; // wrap in ZeroCopyString for neat functions
            zcStr.Trim();
            size_t strLen = zcStr.Length(); // use of lenght here is fast

            const SchemaStringSizeConstrained& fssc = static_cast<const SchemaStringSizeConstrained&>(fieldSchema);
            res = ValidatorResult::Success;
            if (strLen < fssc.minLength) {
                std::string errMsg = std::to_string((unsigned int)fssc.minLength) + "): ";
                errMsg += fieldSchema.name;
                errMsg += " @ "; errMsg += '['; errMsg += sourceObjTypeName; errMsg += ']';
                GlobalLogger.Error(F("String shorter than minLength("), errMsg.c_str());
                res = ValidatorResult::FieldInvalidValue;
                anyError = true;
            }
            if (fssc.maxLength > 0 && strLen > fssc.maxLength) {
                std::string errMsg = std::to_string((unsigned int)fssc.maxLength) + "): ";
                errMsg += fieldSchema.name;
                errMsg += " @ "; errMsg += '['; errMsg += sourceObjTypeName; errMsg += ']';
                GlobalLogger.Error(F("String exceeds maxLength("), errMsg.c_str());
                res = ValidatorResult::FieldInvalidValue;
                anyError = true;
            }
            return res;
        }

        void SchemaStringSizeConstrained::SchemaToJson(const SchemaTypeBase& fieldSchema, std::string& out) {
            SchemaStringBase::SchemaToJson(fieldSchema, out);
            const SchemaStringSizeConstrained& strSchema = static_cast<const SchemaStringSizeConstrained&>(fieldSchema);
            out += ','; ToJsonString::appendNumber(out, "minLength", strSchema.minLength);
            out += ','; ToJsonString::appendNumber(out, "maxLength", strSchema.maxLength);
            if (fieldSchema.type == FieldType::StringSizeConstrained) {
                out += '}'; // this is complete object
            }
        }

        const char* SchemaStringSizeConstrained::GetJavaScriptValidator() {
            return R"rawliteral(

            )rawliteral";
        }

    }

}