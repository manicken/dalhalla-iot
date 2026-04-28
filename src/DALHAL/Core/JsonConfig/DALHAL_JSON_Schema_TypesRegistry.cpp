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

#include "DALHAL_JSON_Schema_TypesRegistry.h"

/** this is the most important as it defines the types available */
#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_FieldType.h> // DALHAL_JsonSchema_FIELD_TYPE_LIST


#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_TypeBase.h>

#include <DALHAL/Core/JsonConfig/Types/Root/DALHAL_JSON_Schema_JsonObjectSchema.h>

#include <DALHAL/Core/JsonConfig/Types/Structures/DALHAL_JSON_Schema_ArrayOfObjects.h>
#include <DALHAL/Core/JsonConfig/Types/Structures/DALHAL_JSON_Schema_ArrayOfPrimitives.h>
#include <DALHAL/Core/JsonConfig/Types/Primitives/DALHAL_JSON_Schema_Bool.h>
#include <DALHAL/Core/JsonConfig/Types/Primitives/DALHAL_JSON_Schema_Float.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/DALHAL_JSON_Schema_HardwarePin.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/DALHAL_JSON_Schema_HardwarePinOrVirtualPin.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/String/DALHAL_JSON_Schema_StringHexBytes.h>
#include <DALHAL/Core/JsonConfig/Types/Primitives/DALHAL_JSON_Schema_Int.h>
#include <DALHAL/Core/JsonConfig/Types/Primitives/DALHAL_JSON_Schema_Number.h>
#include <DALHAL/Core/JsonConfig/Types/Structures/DALHAL_JSON_Schema_Object.h>
#include <DALHAL/Core/JsonConfig/Types/Structures/DALHAL_JSON_Schema_ArrayOfRegistryItems.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/String/DALHAL_JSON_Schema_StringAnyOfArrayConstrained.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/String/DALHAL_JSON_Schema_StringAnyOfByFuncConstrained.h>
#include <DALHAL/Core/JsonConfig/Types/Primitives/DALHAL_JSON_Schema_String.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/String/DALHAL_JSON_Schema_StringSizeConstrained.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/String/DALHAL_JSON_Schema_StringUID.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/String/DALHAL_JSON_Schema_StringUID_Path.h>
#include <DALHAL/Core/JsonConfig/Types/Primitives/DALHAL_JSON_Schema_UInt.h>

#include <DALHAL/Core/JsonConfig/Types/Logical/Groups/DALHAL_JSON_Schema_FieldsGroup.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/Groups/DALHAL_JSON_Schema_AllOfFieldsGroup.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/Groups/DALHAL_JSON_Schema_OneOfFieldsGroup.h>

namespace DALHAL {

    namespace JsonSchema {

        

        const FieldTypeRegistryItem g_fieldTypeTable[] = {
        #define X(name) { #name, Schema##name::RegistryDefine },
            DALHAL_JsonSchema_FIELD_TYPE_LIST
        #undef X
        };

        /*void ForceRegistryLink()
        {
#define X(name) \
            volatile auto force_##name = &Schema##name::RegistryDefine; \
            (void)force_##name;

            DALHAL_JsonSchema_FIELD_TYPE_LIST
#undef X
        }*/

        const FieldTypeRegistryItem& GetFieldTypeRegistryItem(FieldType type) {
            size_t idx = static_cast<size_t>(type);
            if (idx >= static_cast<size_t>(FieldType::_Count_)) {
                static const FieldTypeRegistryItem unknown = {
                    nullptr, {nullptr, nullptr, nullptr, nullptr}
                };
                return unknown;
            }
            return g_fieldTypeTable[idx];
        }

        const char* FieldTypeToString(FieldType type) {
            return GetFieldTypeRegistryItem(type).name;
        }

        void ValidateSchema(const SchemaTypeBase& stb, const char* sourceObjTypeName, bool& anyError) {
            const FieldTypeRegistryItem& regDefItem = GetFieldTypeRegistryItem(stb.type);
            if (regDefItem.name == nullptr) {
                GlobalLogger.Error(F("schema error - could not find schema type @ ValidateSchema"));
                return;
            }
            return regDefItem.define.ValidateSchema(stb, sourceObjTypeName, anyError);
        }

        ValidatorResult ValidateJson(const SchemaTypeBase& stb, const char* sourceObjTypeName, const JsonVariant& jsonObj, bool& anyError) {
            const FieldTypeRegistryItem& regDefItem = GetFieldTypeRegistryItem(stb.type);
            if (regDefItem.name == nullptr) {
                GlobalLogger.Error(F("schema error - could not find schema type @ ValidateJson"));
                return ValidatorResult::SchemaTypeNotFound;
            }
            return regDefItem.define.ValidateJson(stb, sourceObjTypeName, jsonObj, anyError);
        }

        HALValue GetValue(const SchemaTypeBase& stb, const DeviceCreateContext& context) {
            return GetValue(stb, *context.jsonObjItem);
        }

        HALValue GetValue(const SchemaTypeBase& stb, const JsonVariant& jsonObj) {
            const FieldTypeRegistryItem& regDefItem = GetFieldTypeRegistryItem(stb.type);
            if (regDefItem.name == nullptr) {
                GlobalLogger.Error(F("schema error - could not find schema type @ GetValue"));
                return 0;
            }
            return regDefItem.define.GetValue(stb, jsonObj);
        }

        void SchemaToJson(const SchemaTypeBase& stb, std::string& jsonStr) {
            const FieldTypeRegistryItem& regDefItem = GetFieldTypeRegistryItem(stb.type);
            if (regDefItem.name == nullptr) {
                GlobalLogger.Error(F("schema error - could not find schema type @ SchemaToJson"));
                jsonStr += "{}";
                return;
            }
            regDefItem.define.ToJson(stb, jsonStr);
        }

        const char* GetJavaScriptValidator(const SchemaTypeBase& stb) {
            const FieldTypeRegistryItem& regDefItem = GetFieldTypeRegistryItem(stb.type);
            if (regDefItem.name == nullptr) {
                GlobalLogger.Error(F("schema error - could not find schema type @ GetJavaScriptValidator"));
                return "";
            }
            return regDefItem.define.GetJavaScriptValidator();
        }

    }

}