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

#include "DALHAL_JSON_Schema_OneOfFieldsGroup.h"

#include <stdlib.h>
#include <DALHAL/Support/DALHAL_Logger.h>

#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_TypeBase.h>
#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_ValidatorResult.h>

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_TypesRegistry.h>

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_ToJsonStringHelpers.h>

namespace DALHAL {

    namespace JsonSchema {

        constexpr FieldTypeRegistryDefine SchemaOneOfFieldsGroup::RegistryDefine = {
              &ValidateSchema,
              &ValidateJson,
              &SchemaToJson,
              &GetJavaScriptValidator
        };
        
        void SchemaOneOfFieldsGroup::ValidateSchema(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, bool& anyError) {
            auto group = static_cast<const SchemaOneOfFieldsGroup&>(fieldSchema);
            if (group.fields == nullptr) {
                GlobalLogger.Error(F("schema error - SchemaOneOfFieldsGroup fields is nullptr"), sourceObjTypeName);
                anyError = true;
            } else {
                for (size_t i = 0; group.fields[i] != nullptr; ++i) {
                    const SchemaTypeBase& f = *group.fields[i];
                    if (f.type == FieldType::AllOfFieldsGroup || 
                        f.type == FieldType::OneOfFieldsGroup ||
                        f.type == FieldType::FieldsGroup)
                    {
                        // AllOfFieldsGroup and OneOfFieldsGroup collide with the policy
                        // to only find one field, so they cannot be resolved
                        // FieldsGroup can be resolved as it only defines a policy free group
                        // But as it would not solve any special cases we can also skip that case
                        anyError = true;
                        std::string errMsg = f.name?f.name:"nullptr"; errMsg += " @ "; errMsg += sourceObjTypeName?sourceObjTypeName:"nullptr";
                        GlobalLogger.Error(F("schema error - SchemaOneOfFieldsGroup cannot contain any other kind of group"), errMsg.c_str());
                    }
                }
            }
        }

        ValidatorResult SchemaOneOfFieldsGroup::ValidateJson(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, const JsonVariant& jsonObj, bool& anyError) {
            auto group = static_cast<const SchemaOneOfFieldsGroup&>(fieldSchema);
            int foundCount = 0;
            
            for (size_t i = 0; group.fields[i] != nullptr; ++i) {
                const SchemaTypeBase& f = *group.fields[i];
                // failsafe check not really needed as ValidateSchema should take it
                if (f.type == FieldType::AllOfFieldsGroup || 
                    f.type == FieldType::OneOfFieldsGroup ||
                    f.type == FieldType::FieldsGroup)
                {
                    // AllOfFieldsGroup and OneOfFieldsGroup collide with the policy
                    // to only find one field, so they cannot be resolved
                    // FieldsGroup can be resolved as it only defines a policy free group
                    // But as it would not solve any special cases we can also skip that case
                    continue;
                }
                if (jsonObj.containsKey(f.name) == false) {
                    continue;
                }
                foundCount++;
                const FieldTypeRegistryItem& regDefItem = GetFieldTypeRegistryItem(f.type);
                regDefItem.define.ValidateJson(f, group.name?group.name:sourceObjTypeName, jsonObj, anyError);
            }

            if (foundCount == 0 && group.policy == FieldPolicy::Required) {
                std::string errMsg = group.name;
                errMsg += " @ "; errMsg += sourceObjTypeName;
                errMsg += ' '; serializeCollapsed(jsonObj, errMsg);
                GlobalLogger.Error(F("None of the OneOfGroup fields present: "), errMsg.c_str());
                anyError = true;
                return ValidatorResult::RequiredFieldMissing;
            } else if (foundCount > 1) {
                std::string errMsg = group.name;
                errMsg += " @ "; errMsg += sourceObjTypeName;
                errMsg += ' '; serializeCollapsed(jsonObj, errMsg);
                GlobalLogger.Error(F("Multiple fields of the OneOfGroup present: "), errMsg.c_str());
                anyError = true;
                return ValidatorResult::FieldInvalidValue;
            }

            return ValidatorResult::Success;
        }

        void SchemaOneOfFieldsGroup::SchemaToJson(const SchemaTypeBase& fieldSchema, std::string& out) {
            std::string outTemp;
            SchemaTypeBase::SchemaToJson(fieldSchema, outTemp);

            if (fieldSchema.type == FieldType::OneOfFieldsGroup) { 
                SchemaFieldsGroup::CheckAndAddAsInline(fieldSchema, outTemp);                
            } else {
                outTemp+= ','; SchemaFieldsGroup::BuildFieldsArray(static_cast<const SchemaFieldsGroup&>(fieldSchema), outTemp);
            }
            
            out += outTemp;
        }

        const char* SchemaOneOfFieldsGroup::GetJavaScriptValidator() {
            return R"rawliteral(

            )rawliteral";
        }

    }

}