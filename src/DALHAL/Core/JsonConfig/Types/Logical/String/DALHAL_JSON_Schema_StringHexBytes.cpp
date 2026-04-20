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

#include "DALHAL_JSON_Schema_StringHexBytes.h"

#include <stdlib.h>
#include <DALHAL/Support/DALHAL_Logger.h>

#include <DALHAL/Support/ConvertHelper.h>

#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_TypeBase.h>
#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_ValidatorResult.h>
#include <DALHAL/Core/JsonConfig/Types/Primitives/DALHAL_JSON_Schema_String.h>

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_TypesRegistry.h>

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_ToJsonStringHelpers.h>

namespace DALHAL {

    namespace JsonSchema {

        constexpr FieldTypeRegistryDefine SchemaStringHexBytes::RegistryDefine = {
              &ValidateSchema,
              &ValidateJson,
              &SchemaToJson,
              &GetJavaScriptValidator
        };
        
        void SchemaStringHexBytes::ValidateSchema(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, bool& anyError) {
            auto fs = static_cast<const SchemaStringHexBytes&>(fieldSchema);
            if (fs.byteCount == 0) {
                GlobalLogger.Error(F("schema error - SchemaStringHexBytes byteCount cannot be zero"), sourceObjTypeName);
                anyError = true;
            }
        }

        ValidatorResult SchemaStringHexBytes::ValidateJson(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, const JsonVariant& jsonObj, bool& anyError) {
            ValidatorResult res = SchemaString::ValidateJson(fieldSchema, sourceObjTypeName, jsonObj, anyError);
            if (res != ValidatorResult::Success) {
                return res;
            }

            auto fs = static_cast<const SchemaStringHexBytes&>(fieldSchema);
            const char* cStr = jsonObj[fieldSchema.name].as<const char*>();
            // TODO implement settings for delimiter enforcement
            bool parseOk = Convert::HexToBytes(cStr, nullptr, fs.byteCount);
            if (parseOk == false) {
                std::string errMsg = "fs.byteCount:" + std::to_string(fs.byteCount);
                errMsg += " @ "; errMsg += cStr;
                GlobalLogger.Error(F("validateField HexBytes parse error: "), errMsg.c_str());

                anyError = true;
                res = ValidatorResult::FieldInvalidValue;
            }
            return res;
        }

        void SchemaStringHexBytes::SchemaToJson(const SchemaTypeBase& fieldSchema, std::string& out) {
            SchemaString::SchemaToJson(fieldSchema, out);
            auto fs = static_cast<const SchemaStringHexBytes&>(fieldSchema);
            out += ','; ToJsonString::appendNumber(out, "byteCount", fs.byteCount);

            if (fieldSchema.type == FieldType::StringHexBytes) {
                out += '}'; // add the object finalizer if this is the actual object
            }
        }

        const char* SchemaStringHexBytes::GetJavaScriptValidator() {
            return R"rawliteral(

            )rawliteral";
        }

    }

}