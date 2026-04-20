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

#include "DALHAL_JSON_Schema_Bool.h"

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

        constexpr FieldTypeRegistryDefine SchemaBool::RegistryDefine = {
            &ValidateSchema,
            &ValidateJson,
            &SchemaToJson,
            &GetJavaScriptValidator
        };

        void SchemaBool::ValidateSchema(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, bool& anyError) {
            if (SchemaTypeBase::ValidateSchemaNameNotNull(fieldSchema, sourceObjTypeName) == false) {
                anyError = true;
            }
        }

        ValidatorResult SchemaBool::ValidateJson(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, const JsonVariant& jsonObj, bool& anyError) {
            ValidatorResult vRes = SchemaTypeBase::ValidateFieldPresenceAndPolicy(fieldSchema, sourceObjTypeName, jsonObj, anyError);
            if (vRes != ValidatorResult::Success) {
                return vRes; 
            }

            const JsonVariant& value = jsonObj[fieldSchema.name];
            if (!value.is<bool>()) {
                std::string errStr = fieldSchema.name; errStr += " @ ";
                serializeCollapsed(jsonObj, errStr);
                GlobalLogger.Error(F(" must be boolean: "), errStr.c_str());

                anyError = true;
                return ValidatorResult::FieldTypeMismatch;
            }
            return ValidatorResult::Success;
        }

        void SchemaBool::SchemaToJson(const SchemaTypeBase& fieldSchema, std::string& out) {
            SchemaTypeBase::SchemaToJson(fieldSchema, out);
            auto fs = static_cast<const SchemaBool&>(fieldSchema);
            out += ','; ToJsonString::appendNumber(out, "default", fs.defaultValue);
            
            if (fieldSchema.type == FieldType::Bool) {
                out += '}'; // add the object finalizer if this is the actual object
            }
        }

        const char* SchemaBool::GetJavaScriptValidator() {
            return R"rawliteral(

            )rawliteral";
        }
    
    }

}