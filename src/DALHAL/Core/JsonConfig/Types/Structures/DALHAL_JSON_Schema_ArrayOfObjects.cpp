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

#include "DALHAL_JSON_Schema_ArrayOfObjects.h"

#include <stdlib.h>
#include <DALHAL/Support/DALHAL_Logger.h>

#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_TypeBase.h>
#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_ValidatorResult.h>
#include <DALHAL/Core/JsonConfig/Types/Structures/DALHAL_JSON_Schema_ArrayBase.h>

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_TypesRegistry.h>

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_ToJsonStringHelpers.h>

namespace DALHAL {

    namespace JsonSchema {

        constexpr FieldTypeRegistryDefine SchemaArrayOfObjects::RegistryDefine = {
              &ValidateSchema,
              &ValidateJson,
              &SchemaToJson,
              &GetJavaScriptValidator
        };
        
        void SchemaArrayOfObjects::ValidateSchema(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, bool& anyError) {

        }

        ValidatorResult SchemaArrayOfObjects::ValidateJson(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, const JsonVariant& jsonObj, bool& anyError) {
            ValidatorResult vRes = SchemaTypeBase::ValidateFieldPresenceAndPolicy(fieldSchema, sourceObjTypeName, jsonObj, anyError);
            if (vRes != ValidatorResult::Success) {
                return vRes; 
            }
            vRes = SchemaArrayBase::ValidateJson(fieldSchema, sourceObjTypeName, jsonObj, anyError);
            if (vRes != ValidatorResult::Success) {
                return vRes; 
            }
            const JsonArray& array = jsonObj[fieldSchema.name].as<JsonArray>();
            auto fs = static_cast<const SchemaArrayOfObjects&>(fieldSchema);

            for (JsonVariant item : array) {
                if (item.is<const char*>()) continue; // comment item
                if (!item.is<JsonObject>()) {
                    GlobalLogger.Error(F("Field is not an object:"), fieldSchema.name);
                    anyError = true;
                    continue;
                }
                JsonObjectSchema::ValidateJson(fs.subtype, fieldSchema.name, item, anyError);
            }

            return ValidatorResult::Success;
        }

        void SchemaArrayOfObjects::SchemaToJson(const SchemaTypeBase& fieldSchema, std::string& out) {
            SchemaTypeBase::SchemaToJson(fieldSchema, out);
            auto fs = static_cast<const SchemaArrayOfObjects&>(fieldSchema);
            out += ','; ToJsonString::appendKey(out, "subtype");
            //out += '{'; SchemaToJson adds it
            if (Gui::HaveUseInline(fieldSchema.guiFlags)) {
                if (ToJsonString::inlinesContains(fs.subtype->typeName) == false) {
                    std::string subTypeOutTemp;
                    JsonObjectSchema::SchemaToJson(fs.subtype, subTypeOutTemp);
                    ToJsonString::addToInlines(fs.subtype->typeName, subTypeOutTemp);
                }
                ToJsonString::appendQuoted(out, fs.subtype->typeName);
                
            } else {
                JsonObjectSchema::SchemaToJson(fs.subtype, out);
            }
            //out += '}'; SchemaToJson adds it

            if (fs.renderAllAllowedValuesFromStringConstraint != nullptr) {
                out += ','; ToJsonString::appendString(out, "renderAllAllowedValuesFromStringConstraint", fs.renderAllAllowedValuesFromStringConstraint->name);
            }
            
            if (fieldSchema.type == FieldType::ArrayOfObjects) { 
                out += '}'; // add the object finalizer if this is the actual object
            }
        }

        const char* SchemaArrayOfObjects::GetJavaScriptValidator() {
            return R"rawliteral(

            )rawliteral";
        }

        const JsonArray& SchemaArrayOfObjects::GetValidatedJsonArray(const SchemaArrayOfObjects& saoo, const JsonVariant& jsonObj) {
            return jsonObj[saoo.name].as<JsonArray>();
        }

    }

}