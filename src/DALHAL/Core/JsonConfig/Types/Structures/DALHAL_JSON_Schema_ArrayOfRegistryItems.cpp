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

#include "DALHAL_JSON_Schema_ArrayOfRegistryItems.h"

#include <stdlib.h>
#include <DALHAL/Support/DALHAL_Logger.h>

#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_TypeBase.h>
#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_ValidatorResult.h>
#include <DALHAL/Core/JsonConfig/Types/Primitives/DALHAL_JSON_Schema_String.h>
#include <DALHAL/Core/JsonConfig/Types/Structures/DALHAL_JSON_Schema_ArrayBase.h>

#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Base.h>

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_TypesRegistry.h>

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_ToJsonStringHelpers.h>

namespace DALHAL {

    namespace JsonSchema {

        constexpr FieldTypeRegistryDefine SchemaArrayOfRegistryItems::RegistryDefine = {
              &SchemaValidate,
              &ValidateJson,
              &SchemaToJson,
              &GetJavaScriptValidator
        };
        
        void SchemaArrayOfRegistryItems::SchemaValidate(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, bool& anyError) {

        }

        ValidatorResult SchemaArrayOfRegistryItems::ValidateJson(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, const JsonVariant& jsonObj, bool& anyError) {
            ValidatorResult vRes = SchemaTypeBase::ValidateFieldPresenceAndPolicy(fieldSchema, sourceObjTypeName, jsonObj, anyError);
            if (vRes != ValidatorResult::Success) {
                return vRes; 
            }
            vRes = SchemaArrayBase::ValidateJson(fieldSchema, sourceObjTypeName, jsonObj, anyError);
            if (vRes != ValidatorResult::Success) {
                return vRes; 
            }
            const JsonArray& items = jsonObj[fieldSchema.name].as<JsonArray>();
            auto fs = static_cast<const SchemaArrayOfRegistryItems&>(fieldSchema);

            uint32_t itemCount = items.size();
            const Registry::Item* reg = fs.subtypes;
            
            for (uint32_t i = 0; i < itemCount; ++i) {
                const JsonVariant& jsonItem = items[i];
                if (jsonItem.is<const char*>()) { continue; } // comment item
                // TODO make this optional so that we can validate disabled items as well
                if (DALHAL::Device::DisabledInJson(jsonItem)) { continue; } // disabled

                // first we need to validate the type field
                bool anyErrorTemp = false;
                SchemaString::ValidateJson(JsonSchema::typeField, sourceObjTypeName, jsonItem, anyErrorTemp);
                if (anyErrorTemp == true) {
                    anyError = true;
                    continue; // skip the current json device
                }
                const char* type_cStr = jsonItem[DALHAL_COMMON_CFG_NAME_TYPE];
                const Registry::Item& regItem = Registry::GetItem(reg, type_cStr);
                if (regItem.typeName == nullptr) {
                    GlobalLogger.Error(F("could not find type:"),type_cStr);
                    anyError = true;
                    continue; // skip the current json device
                }

                if (regItem.def == nullptr) {
                    GlobalLogger.Error(F("FATAL regitem.def == nullptr"));
                    anyError = true;
                    continue; // skip the current json device
                }
                
                if (regItem.def->jsonSchema == nullptr) {
                    GlobalLogger.Error(F("FATAL regItem.def->jsonSchema == nullptr"));

                    anyError = true;
                    continue; // skip the current json device
                }
                JsonObjectSchema::ValidateJson(regItem.def->jsonSchema, reg->typeName, jsonItem, anyError);
            }

            return ValidatorResult::Success;
        }

        void SchemaArrayOfRegistryItems::SchemaToJson(const SchemaTypeBase& fieldSchema, std::string& out) {

            // dont forget to change type here to the correct one
            if (fieldSchema.type == FieldType::ArrayOfRegistryItems) { 
                out += '}'; // add the object finalizer if this is the actual object
            }
        }

        const char* SchemaArrayOfRegistryItems::GetJavaScriptValidator() {
            return R"rawliteral(

            )rawliteral";
        }

    }

}