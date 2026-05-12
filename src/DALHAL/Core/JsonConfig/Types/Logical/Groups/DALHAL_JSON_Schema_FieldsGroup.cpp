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

#include "DALHAL_JSON_Schema_FieldsGroup.h"

#include <stdlib.h>
#include <DALHAL/Support/DALHAL_Logger.h>

#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_TypeBase.h>
#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_ValidatorResult.h>

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_TypesRegistry.h>

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_ToJsonStringHelpers.h>

namespace DALHAL {

    namespace JsonSchema {

        __attribute__((used, externally_visible))
        constexpr FieldTypeRegistryDefine SchemaFieldsGroup::RegistryDefine = {
              &ValidateSchema,
              &ValidateJson,
              &SchemaToJson,
              &GetJavaScriptValidator
        };
        //volatile const void* keep_SchemaFieldsGroup = &DALHAL::JsonSchema::SchemaFieldsGroup::RegistryDefine;
        
        void SchemaFieldsGroup::ValidateSchema(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, bool& anyError) {
            auto group = static_cast<const SchemaFieldsGroup&>(fieldSchema);
            if (group.fields == nullptr) {
                GlobalLogger.Error(F("schema error - SchemaFieldsGroup fields is nullptr"), sourceObjTypeName?sourceObjTypeName:"nullptr");
                anyError = true;
            } 
        }

        ValidatorResult SchemaFieldsGroup::ValidateJson(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, const JsonVariant& jsonObj, bool& anyError) {
            auto group = static_cast<const SchemaFieldsGroup&>(fieldSchema);

            for (size_t i = 0; group.fields[i] != nullptr; ++i) {
                //const SchemaTypeBase& f = *group.fields[i];

                JsonSchema::ValidateJson(*group.fields[i], group.name?group.name:sourceObjTypeName, jsonObj, anyError);
                //const FieldTypeRegistryItem& regDefItem = GetFieldTypeRegistryItem(f.type);
                //regDefItem.define.ValidateJson(f, group.name?group.name:sourceObjTypeName, jsonObj, anyError);
            }
            return ValidatorResult::Success;
        }

        void SchemaFieldsGroup::BuildFieldsArray(const SchemaFieldsGroup& group, std::string& out)
        {
            ToJsonString::appendKey(out, F("fields"));
            out += '[';
            for (int i = 0; group.fields[i] != nullptr; ++i) {
                if (i > 0) out += ",";

                const SchemaTypeBase& field = *group.fields[i];

                JsonSchema::SchemaToJson(field, out); // shortcut and safer to use, from DALHAL_JSON_Schema_TypesRegistry.h
                //const auto& regDefItem = GetFieldTypeRegistryItem(field.type);
                //regDefItem.define.ToJson(field, out);
            }
            out += ']';
        }
        /** this should only be used on final object */
        void SchemaFieldsGroup::CheckAndAddAsInline(const SchemaTypeBase& fieldSchema, std::string& out) {
            
            if (Gui::HaveUseInline(fieldSchema.guiFlags)) {
                if (ToJsonString::inlinesContains(fieldSchema.name) == false) {
                    std::string inlineStr;
                    SchemaTypeBase::SchemaToJson(fieldSchema, inlineStr);
                    inlineStr += ','; BuildFieldsArray(static_cast<const SchemaFieldsGroup&>(fieldSchema), inlineStr);
                    inlineStr += '}'; // add the object finalizer
                    ToJsonString::addToInlines(fieldSchema.name, inlineStr);
                }
                out += '{';
                ToJsonString::appendString(out, F("type"), F("_inline_"));
                out += ','; ToJsonString::appendString(out, F("name"), fieldSchema.name);

            } else {
                SchemaTypeBase::SchemaToJson(fieldSchema, out);
                out += ','; BuildFieldsArray(static_cast<const SchemaFieldsGroup&>(fieldSchema), out);
            }
            out += '}'; // add the object finalizer
        }

        void SchemaFieldsGroup::SchemaToJson(const SchemaTypeBase& fieldSchema, std::string& out) {
            if (fieldSchema.type == FieldType::FieldsGroup) { 
                SchemaFieldsGroup::CheckAndAddAsInline(fieldSchema, out);
                
            } else {
                SchemaTypeBase::SchemaToJson(fieldSchema, out);
                out += ','; SchemaFieldsGroup::BuildFieldsArray(static_cast<const SchemaFieldsGroup&>(fieldSchema), out);
            }
        }

        const char* SchemaFieldsGroup::GetJavaScriptValidator() {
            return R"rawliteral(

            )rawliteral";
        }

    }

}