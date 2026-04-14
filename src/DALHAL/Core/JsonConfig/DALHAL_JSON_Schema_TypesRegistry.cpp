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

#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_TypeBase.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_ComplexTypes.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_JsonObjectSchema.h>

#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_Array.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_ArrayPrimitive.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_Bool.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_Float.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_HardwarePin.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_HardwarePinOrVirtualPin.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_HexBytes.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_Int.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_Number.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_Object.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_RegistryArray.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_StringAnyOfArrayConstrained.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_StringAnyOfByFuncConstrained.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_StringBase.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_StringSizeConstrained.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_StringUID.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_StringUID_Path.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_UInt.h>

namespace DALHAL {

    namespace JsonSchema {

        const char* ValidatorResultToString(ValidatorResult res) {
            switch (res)
            {
                case ValidatorResult::FieldEmpty: return "FieldEmpty";
                case ValidatorResult::FieldInvalidValue: return "FieldInvalidValue";
                case ValidatorResult::FieldTypeMismatch: return "FieldTypeMismatch";
                case ValidatorResult::RequiredFieldMissing: return "RequiredFieldMissing";
                case ValidatorResult::Success: return "Success";
                default: return "Unknown";
            }
        }

        const FieldTypeRegistryItem g_fieldTypeTable[] = {
        #define X(name) { #name, Schema##name::RegistryDefine },
            DALHAL_JsonSchema_FIELD_TYPE_LIST
        #undef X
        };

        const FieldTypeRegistryItem& GetFieldTypeRegistryItem(FieldType type) {
            size_t idx = static_cast<size_t>(type);
            if (idx >= static_cast<size_t>(FieldType::_Count_)) {
                static const FieldTypeRegistryItem unknown = {
                    "Unknown", {nullptr, nullptr, nullptr, nullptr}
                };
                return unknown;
            }
            return g_fieldTypeTable[idx];
        }

        const char* FieldTypeToString(FieldType type) {
            return GetFieldTypeRegistryItem(type).name;
        }

    }

}