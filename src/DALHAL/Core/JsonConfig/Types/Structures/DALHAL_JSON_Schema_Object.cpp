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

#include "DALHAL_JSON_Schema_Object.h"

#include <stdlib.h>
#include <DALHAL/Support/DALHAL_Logger.h>

#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_TypeBase.h>
#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_ValidatorResult.h>
#include <DALHAL/Core/JsonConfig/Types/Root/DALHAL_JSON_Schema_JsonObjectSchema.h>

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_TypesRegistry.h>

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_ToJsonStringHelpers.h>

namespace DALHAL {

    namespace JsonSchema {

        constexpr FieldTypeRegistryDefine SchemaObject::RegistryDefine = {
              &ValidateSchema,
              &ValidateJson,
              &SchemaToJson,
              &GetJavaScriptValidator
        };
        
        void SchemaObject::ValidateSchema(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, bool& anyError) {
            if (SchemaTypeBase::ValidateSchemaNameNotNull(fieldSchema, sourceObjTypeName)) {
                anyError = true;
            }
            auto fs = static_cast<const SchemaObject&>(fieldSchema);
            if (fs.subtype == nullptr) {
                GlobalLogger.Error(F("schema error - SchemaObject subtype == nullptr"), sourceObjTypeName);
                anyError = true;
            } else {
                JsonObjectSchema::ValidateSchema(fs.subtype, sourceObjTypeName, anyError);
            }
        }

        ValidatorResult SchemaObject::ValidateJson(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, const JsonVariant& jsonObj, bool& anyError) {
            ValidatorResult vRes = SchemaTypeBase::ValidateFieldPresenceAndPolicy(fieldSchema, sourceObjTypeName, jsonObj, anyError);
            if (vRes != ValidatorResult::Success) {
                return vRes; 
            }
            return JsonObjectSchema::ValidateJson(static_cast<const SchemaObject&>(fieldSchema).subtype, sourceObjTypeName, jsonObj[fieldSchema.name], anyError);
        }

        void SchemaObject::SchemaToJson(const SchemaTypeBase& fieldSchema, std::string& out) {

            // dont forget to change type here to the correct one
            if (fieldSchema.type == FieldType::Object) { 
                out += '}'; // add the object finalizer if this is the actual object
            }
        }

        const char* SchemaObject::GetJavaScriptValidator() {
            return R"rawliteral(

            )rawliteral";
        }

    }

}