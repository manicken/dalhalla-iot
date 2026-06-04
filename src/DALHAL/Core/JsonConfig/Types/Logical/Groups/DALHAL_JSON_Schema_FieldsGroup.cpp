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

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_ToJsonString.h>

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

        void SchemaFieldsGroup::BuildFieldsArray(const SchemaFieldsGroup& group, StringBuilderStreamer& sbs, SchemaEmitMode mode)
        {
            sbs.write_jsonKey(F("fields"));
            sbs.write_json_array_begin();
            for (int i = 0; group.fields[i] != nullptr; ++i) {
                if (i > 0) sbs.write_json_value_separator();

                const SchemaTypeBase& field = *group.fields[i];

                JsonSchema::SchemaToJson(field, sbs, SchemaEmitMode::ByReference); // shortcut and safer to use, from DALHAL_JSON_Schema_TypesRegistry.h
                //const auto& regDefItem = GetFieldTypeRegistryItem(field.type);
                //regDefItem.define.ToJson(field, out);
            }
            sbs.write_json_array_end();
        }
        /** this should only be used on final object */
        void SchemaFieldsGroup::CheckAndAddAsInline(const SchemaTypeBase& fieldSchema, StringBuilderStreamer& sbs, SchemaEmitMode mode) {
            
            if (Gui::HaveUseInline(fieldSchema.guiFlags) && mode == SchemaEmitMode::ByReference) {
                if (ToJsonString::ByReferenceContains(fieldSchema.name) == false) {
                    // just add here to be generated later
                    ToJsonString::addToByReference(fieldSchema.name, fieldSchema);
                }
                sbs.write_json_object_begin();
                sbs.write_jsonString(F("type"), F("_byref_"));
                sbs.write_json_value_separator(); sbs.write_jsonString(F("name"), fieldSchema.name);

            } else {
                SchemaTypeBase::SchemaToJson(fieldSchema, sbs, mode);
                sbs.write_json_value_separator(); BuildFieldsArray(static_cast<const SchemaFieldsGroup&>(fieldSchema), sbs, mode);
            }
            sbs.write_json_object_end(); // add the object finalizer
        }

        void SchemaFieldsGroup::SchemaToJson(const SchemaTypeBase& fieldSchema, StringBuilderStreamer& sbs, SchemaEmitMode mode) {
            if (fieldSchema.type == FieldType::FieldsGroup) { 
                SchemaFieldsGroup::CheckAndAddAsInline(fieldSchema, sbs, mode);
                
            } else {
                SchemaTypeBase::SchemaToJson(fieldSchema, sbs, mode);
                sbs.write_json_value_separator(); SchemaFieldsGroup::BuildFieldsArray(static_cast<const SchemaFieldsGroup&>(fieldSchema), sbs, mode);
            }
        }

        const char* SchemaFieldsGroup::GetJavaScriptValidator() {
            return R"rawliteral(

            )rawliteral";
        }

    }

}