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

#include "DALHAL_JSON_Schema_TypeBase.h"

#include <ArduinoJson.h>
#include <DALHAL/Core/JsonConfig/DALHAL_ArduinoJSON_ext.h>

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_TypesRegistry.h>

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_ToJsonStringHelpers.h>

#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_FieldPolicy.h>

namespace DALHAL {

    namespace JsonSchema {

        HALValue SchemaTypeBase::ExtractViaRegistryFrom(const JsonVariant& jsonObj) const {
            return JsonSchema::GetValue(*this, jsonObj);
        }

        bool SchemaTypeBase::ValidateSchemaNameNotNull(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName) {
            if (fieldSchema.name == nullptr) {
                GlobalLogger.Error(F("invalid schema field - name cannot be nullptr @ "), sourceObjTypeName);
                return false;
            }
            return true;
        }

        ValidatorResult SchemaTypeBase::ValidateFieldPresenceAndPolicy(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, const JsonVariant& jsonObj, bool& anyError) {
            bool exists = jsonObj.containsKey(fieldSchema.name);
            if ((exists == false) && (fieldSchema.policy == FieldPolicy::Required)) {
                std::string errMsg = fieldSchema.name; 
                errMsg += " @ "; errMsg += '['; errMsg += sourceObjTypeName; errMsg += ']';
                errMsg += ' ';
                serializeCollapsed(jsonObj, errMsg);
                GlobalLogger.Error(F("Required field missing: "), errMsg.c_str());
                
                anyError = true;
                return ValidatorResult::RequiredFieldMissing;
            }
            return (exists)?(ValidatorResult::Success):(ValidatorResult::FieldMissing);
        }
        
        void SchemaTypeBase::SchemaToJson(const SchemaTypeBase& schema, std::string& out) {
            out += '{'; // this is allways added
            const char* type_cStr = FieldTypeToString(schema.type);
            //const char* type_cStr = "test";
            ToJsonString::appendString(out, F("type"), type_cStr?type_cStr:"nullptr unknown");
            out += ','; ToJsonString::appendString(out, F("name"), schema.name);
            out += ','; ToJsonString::appendBool(out, F("required"), (schema.policy == FieldPolicy::Required));
            if (Gui::HaveAnyNotIncludingInline(schema.guiFlags)) {
                out += ','; ToJsonString::appendKey(out, F("gui")); Gui::ToJson(schema.guiFlags, out);
            }
        }
    }

}