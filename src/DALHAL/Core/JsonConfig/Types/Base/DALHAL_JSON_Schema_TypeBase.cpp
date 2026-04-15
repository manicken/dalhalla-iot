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

        bool SchemaTypeBase::SchemaValidateNameNotNull(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName) {
            if (fieldSchema.name == nullptr) {
                GlobalLogger.Error(F("invalid schema field - name cannot be nullptr @ "), sourceObjTypeName);
                return false;
            }
            return true;
        }

        ValidatorResult SchemaTypeBase::ValidateJson(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, const JsonVariant& jsonObj, bool& anyError) {
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
            return ValidatorResult::Success;
        }
        
        void SchemaTypeBase::SchemaToJson(const SchemaTypeBase& schema, std::string& out) {
            out += '{'; // this is allways added
            ToJsonString::appendString(out, "type", FieldTypeToString(schema.type));
            out += ','; ToJsonString::appendString(out, "name", schema.name);
            out += ','; ToJsonString::appendBool(out, "required", (schema.policy == FieldPolicy::Required));
            out += ','; ToJsonString::appendKey(out, "gui"); out += '{';
            if (Gui::hasFlag(schema.guiFlags, Gui::DisableByDefault)) {
                out += ",\"DisableByDefault\":true";
            }
            if (Gui::hasFlag(schema.guiFlags, Gui::HideLabel)) {
                out += ",\"HideLabel\":true";
            }
            if (Gui::hasFlag(schema.guiFlags, Gui::ReadOnly)) {
                out += ",\"ReadOnly\":true";
            }
            if (Gui::hasFlag(schema.guiFlags, Gui::RenderAllAllowedValues)) {
                out += ",\"RenderAllAllowedValues\":true";
            }
            // this is actually a SchemaToJson parameter but we can include it here for clarity
            if (Gui::hasFlag(schema.guiFlags, Gui::UseInline)) {
                out += ",\"UseInline\":true";
            }
            out += '}';
        }
    }

}