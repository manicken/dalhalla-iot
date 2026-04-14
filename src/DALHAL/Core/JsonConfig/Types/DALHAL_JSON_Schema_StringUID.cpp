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

#pragma once

#include "DALHAL_JSON_Schema_StringUID.h"

#include <stdlib.h>

#include <DALHAL/Core/Types/DALHAL_UID.h>

#include <DALHAL/Support/DALHAL_Logger.h>

#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_TypeBase.h>
#include <DALHAL/Core/JsonConfig/Types/Primitives/DALHAL_JSON_Schema_StringBase.h>

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_TypesRegistry.h>

namespace DALHAL {

    namespace JsonSchema {

        constexpr FieldTypeRegistryDefine SchemaStringUID::RegistryDefine = {
              &SchemaValidate,
              &ValidateJson,
              &SchemaToJson,
              JavaScriptValidator
        };

        void SchemaStringUID::SchemaValidate(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, bool& anyError) {
            SchemaStringBase::SchemaValidate(fieldSchema, sourceObjTypeName, anyError);
        }

        ValidatorResult SchemaStringUID::ValidateJson(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, const JsonVariant& jsonObj, bool& anyError) {
            ValidatorResult res = SchemaStringBase::ValidateJson(fieldSchema, sourceObjTypeName, jsonObj, anyError);

            if (res != ValidatorResult::Success) {
                return res; //  this mean either this field dont exist or that it's not a valid string 
                // so it can't continue 
            }

            const char* cStr = jsonObj[fieldSchema.name].as<const char*>(); // this is now safe
            ZeroCopyString zcStr = cStr; // wrap in ZeroCopyString for neat functions
            zcStr.Trim();
            size_t strLen = zcStr.Length(); // use of lenght here is fast
            
            if (strLen > HAL_UID::Size) {
                GlobalLogger.Error(F("SchemaStringUID - is too long"));
                anyError = true;
                return ValidatorResult::FieldInvalidValue;
            }
            // TODO do proper check if UID is allready defined
            // but now pass on all paths
            return ValidatorResult::Success;
        }

        void SchemaStringUID::SchemaToJson(const SchemaTypeBase& fieldSchema, std::string& out) {
            SchemaStringBase::SchemaToJson(fieldSchema, out);
            if (fieldSchema.type == FieldType::StringUID_Path) {
                out += '}'; // this is complete object
            }
        }

        const char* SchemaStringUID::JavaScriptValidator = R"rawliteral(

        )rawliteral";

    }

}