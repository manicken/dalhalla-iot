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

#include "DALHAL_JSON_Schema_ArrayOfPrimitives.h"

#include <stdlib.h>
#include <DALHAL/Support/DALHAL_Logger.h>

#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_TypeBase.h>
#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_ValidatorResult.h>
#include <DALHAL/Core/JsonConfig/Types/Structures/DALHAL_JSON_Schema_ArrayBase.h>

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_TypesRegistry.h>

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_ToJsonStringHelpers.h>

#include <DALHAL/Support/ConvertHelper.h>

namespace DALHAL {

    namespace JsonSchema {

        constexpr FieldTypeRegistryDefine SchemaArrayOfPrimitives::RegistryDefine = {
              &ValidateSchema,
              &ValidateJson,
              &SchemaToJson,
              &GetJavaScriptValidator
        };
        
        void SchemaArrayOfPrimitives::ValidateSchema(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, bool& anyError) {

        }

        ValidatorResult SchemaArrayOfPrimitives::ValidateJson(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, const JsonVariant& jsonObj, bool& anyError) {
            ValidatorResult vRes = SchemaTypeBase::ValidateFieldPresenceAndPolicy(fieldSchema, sourceObjTypeName, jsonObj, anyError);
            if (vRes != ValidatorResult::Success) {
                return vRes; 
            }
            vRes = SchemaArrayBase::ValidateJson(fieldSchema, sourceObjTypeName, jsonObj, anyError);
            if (vRes != ValidatorResult::Success) {
                return vRes; 
            }
            const JsonArray& array = jsonObj[fieldSchema.name].as<JsonArray>();
            auto fs = static_cast<const SchemaArrayOfPrimitives&>(fieldSchema);
            // Validate each element against primitiveTypeFlags
            for (JsonVariant item : array) {
                bool valid = false;

                if (item.is<int>()) {
                    valid = fs.primitiveTypeFlags & PrimitiveTypeFlags::AllowInt;
                } else if (item.is<unsigned int>()) {
                    valid = fs.primitiveTypeFlags & PrimitiveTypeFlags::AllowUInt;
                } else if (item.is<float>() || item.is<double>()) {
                    valid = fs.primitiveTypeFlags & PrimitiveTypeFlags::AllowFloat;
                } else if (item.is<bool>()) {
                    valid = fs.primitiveTypeFlags & PrimitiveTypeFlags::AllowBool;
                }

                if (!valid) {
                    GlobalLogger.Error(F("Array element has invalid type"), fs.name);
                    anyError = true;
                }
            }
            

            return ValidatorResult::Success;
        }

        void SchemaArrayOfPrimitives::SchemaToJson(const SchemaTypeBase& fieldSchema, std::string& out) {
            SchemaTypeBase::SchemaToJson(fieldSchema, out);
            auto fs = static_cast<const SchemaArrayOfPrimitives&>(fieldSchema);
            // TODO do proper convertion into bool fields
            std::string primitiveTypeFlagsHex = Convert::toHex(fs.primitiveTypeFlags);
            out += ','; ToJsonString::appendString(out, "primitiveTypeFlags", primitiveTypeFlagsHex.c_str());
            
            if (fieldSchema.type == FieldType::ArrayOfPrimitives) { 
                out += '}'; // add the object finalizer if this is the actual object
            }
        }

        const char* SchemaArrayOfPrimitives::GetJavaScriptValidator() {
            return R"rawliteral(

            )rawliteral";
        }

        bool SchemaArrayOfPrimitives::ExtractValues(const SchemaArrayOfPrimitives& fieldSchema, const JsonVariant& jsonObj, HALValue** outValues, int& valueCount)
        {
            if (jsonObj.containsKey(fieldSchema.name) == false) {
                return false;
            }
            const JsonVariant& jsonFieldObj = jsonObj[fieldSchema.name];
            if (jsonFieldObj.is<JsonArray>() == false) {
                return false;
            }
            const JsonArray arr = jsonFieldObj.as<JsonArray>();
            if (arr.size() == 0) {
                return false;
            }

            if (outValues == nullptr) {
                GlobalLogger.Error(F("SchemaArrayOfPrimitives::ExtractValues - outValues is nullptr"));
                return false;
            }

            size_t arraySize = arr.size();

            HALValue* out = new HALValue[arraySize];
            *outValues = out;
            valueCount = arraySize;

            for (int i = 0; i < (int)arraySize; ++i) {
                const JsonVariant& item = arr[i];
                HALValue hVal;
                if (item.is<float>()) {
                    hVal.set(item.as<float>());
                } else if (item.is<signed int>()) {
                    hVal.set(item.as<signed int>());
                } else if (item.is<unsigned int>()) {
                    hVal.set(item.as<unsigned int>());
                } else if (item.is<bool>()) {
                    hVal.set(item.as<bool>());
                } else if (item.is<const char*>()) {
                    hVal.set(item.as<const char*>());
                } // else hVal is set to unset
                out[i] = hVal;
            }

            return true;
        }

    }

}