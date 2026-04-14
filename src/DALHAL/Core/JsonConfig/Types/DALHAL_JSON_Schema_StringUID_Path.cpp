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

#include "DALHAL_JSON_Schema_StringUID_Path.h"

namespace DALHAL {

    namespace JsonSchema {
        
        constexpr FieldTypeRegistryDefine SchemaStringUID_Path::RegistryDefine = {
              &SchemaValidate,
              &ValidateJson,
              &SchemaToJson,
              &GetJavaScriptValidator
        };

        void SchemaStringUID_Path::SchemaValidate(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, bool& anyError) {
            SchemaStringBase::SchemaValidate(fieldSchema, sourceObjTypeName, anyError);
        }

        ValidatorResult SchemaStringUID_Path::ValidateJson(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, const JsonVariant& jsonObj, bool& anyError) {
            ValidatorResult res = SchemaStringBase::ValidateJson(fieldSchema, sourceObjTypeName, jsonObj, anyError);

            if (res != ValidatorResult::Success) {
                return res; //  this mean either this field dont exist or that it's not a valid string 
                // so it can't continue 
            }

            const char* cStr = jsonObj[fieldSchema.name].as<const char*>(); // this is now safe
            ZeroCopyString zcStr = cStr; // wrap in ZeroCopyString for neat functions
            zcStr.Trim();
            
            // TODO do proper check of UID_Path
            // but now pass on all paths
            return ValidatorResult::Success;
        }

        void SchemaStringUID_Path::SchemaToJson(const SchemaTypeBase& fieldSchema, std::string& out) {
            SchemaStringBase::SchemaToJson(fieldSchema, out);
            if (fieldSchema.type == FieldType::StringUID_Path) {
                out += '}'; // this is complete object
            }
        }

        const char* SchemaStringUID_Path::GetJavaScriptValidator() {
            return R"rawliteral(

            )rawliteral";
        }
    }

}