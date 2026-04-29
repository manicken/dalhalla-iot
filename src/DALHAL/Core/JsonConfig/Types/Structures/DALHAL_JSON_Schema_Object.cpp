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

#include "DALHAL_JSON_Schema_Object.h"

#include <stdlib.h>
#include <DALHAL/Support/DALHAL_Logger.h>

#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_TypeBase.h>
#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_ValidatorResult.h>
#include <DALHAL/Core/JsonConfig/Types/Root/DALHAL_JSON_Schema_JsonObjectSchema.h>

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_TypesRegistry.h>

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_ToJsonStringHelpers.h>

namespace DALHAL {

    namespace JsonSchema {

        __attribute__((used, externally_visible))
        constexpr FieldTypeRegistryDefine SchemaObject::RegistryDefine = {
              &ValidateSchema,
              &ValidateJson,
              &SchemaToJson,
              &GetJavaScriptValidator
        };
        //volatile const void* keep_SchemaObject = &DALHAL::JsonSchema::SchemaObject::RegistryDefine;
        
        void SchemaObject::ValidateSchema(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, bool& anyError) {
            if (SchemaTypeBase::ValidateSchemaNameNotNull(fieldSchema, sourceObjTypeName)) {
                anyError = true;
            }
            auto fs = static_cast<const SchemaObject&>(fieldSchema);
            if (fs.subtype == nullptr) {
                GlobalLogger.Error(F("schema error - SchemaObject subtype == nullptr"), sourceObjTypeName);
                anyError = true;
            } else {
                JsonObjectSchema::ValidateSchema(fs.subtype, sourceObjTypeName, anyError);
            }
        }

        ValidatorResult SchemaObject::ValidateJson(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, const JsonVariant& jsonObj, bool& anyError) {
            ValidatorResult vRes = SchemaTypeBase::ValidateFieldPresenceAndPolicy(fieldSchema, sourceObjTypeName, jsonObj, anyError);
            if (vRes != ValidatorResult::Success) {
                return vRes; 
            }
            const JsonVariant& subJsonObj = jsonObj[fieldSchema.name];
            if (!subJsonObj.is<JsonObject>()) {
                GlobalLogger.Error(F("Field is not an object:"), fieldSchema.name);
                anyError = true;
                return ValidatorResult::FieldTypeMismatch;
            }
            const SchemaObject& fs = static_cast<const SchemaObject&>(fieldSchema);

            return JsonObjectSchema::ValidateJson(fs.subtype, sourceObjTypeName, subJsonObj, anyError);
        }

        void SchemaObject::SchemaToJson(const SchemaTypeBase& fieldSchema, std::string& out) {
            SchemaTypeBase::SchemaToJson(fieldSchema, out);
            auto fs = static_cast<const SchemaObject&>(fieldSchema);
            out += ','; ToJsonString::appendKey(out, "subtype");
            //out += '{'; SchemaToJson adds it
            JsonObjectSchema::SchemaToJson(fs.subtype, out);
            //out += '}'; SchemaToJson adds it
            if (fieldSchema.type == FieldType::Object) { 
                out += '}'; // add the object finalizer if this is the actual object
            }
        }

        const char* SchemaObject::GetJavaScriptValidator() {
            return R"rawliteral(

            )rawliteral";
        }

        void ApplyToStruct(size_t structOffset, const HALValue& val, void* outStruct)
        {
            uint8_t* base = static_cast<uint8_t*>(outStruct);
            void* target = base + structOffset;

            // as we do actually check the halvalue type explicit
            // we can use the raw getters
            switch (val.getType()) {
                case HALValue::Type::UINT:
                    *static_cast<uint32_t*>(target) = val.asRawUInt();
                    break;

                case HALValue::Type::INT:
                    *static_cast<int32_t*>(target) = val.asRawInt();
                    break;

                case HALValue::Type::BOOL:
                    *static_cast<bool*>(target) = val.asRawBool();
                    break;

                case HALValue::Type::FLOAT:
                    *static_cast<float*>(target) = val.asRawFloat();
                    break;

                case HALValue::Type::CSTRING:
                    *static_cast<const char**>(target) = val.asRawConstChar();
                    break;

                default:
                    GlobalLogger.Error(F("Unsupported HALValue type in ApplyToStruct"));
                    break;
            }
        }

        bool SchemaObject::ExtractValues(const SchemaObject& schemaField, const JsonVariant& jsonObj, void* outStruct)
        {
            if (schemaField.subtype == nullptr) {
                GlobalLogger.Error(F("schema error - schemaField.subtype == nullptr @ SchemaObject::ExtractValues"));
                return false; // should never happend
            }
            const JsonObjectSchema& jsonObjSchema = *schemaField.subtype;
            if (jsonObj.containsKey(schemaField.name) == false) {
                return false; // optional field, as required fields fail on prevalidation
            }
            const JsonVariant& jsonSubObject = jsonObj[schemaField.name];
            for (int i = 0; jsonObjSchema.fields[i] != nullptr; ++i) {
                const SchemaTypeBase& field = *jsonObjSchema.fields[i];

                HALValue val = GetValue(field, jsonSubObject); // this return default values if the field themselves are missing

                ApplyToStruct(field.structOffset, val, outStruct);
            }
            return true;
        }

        const JsonObject SchemaObject::GetValidatedJsonObject(const SchemaObject& schemaField, const JsonVariant& jsonObj) {
            return jsonObj[schemaField.name].as<JsonObject>();
        }

    }

}