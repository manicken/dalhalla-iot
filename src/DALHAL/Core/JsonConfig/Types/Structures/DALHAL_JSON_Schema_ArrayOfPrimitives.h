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

#include <stdlib.h>
#include <DALHAL/Support/DALHAL_Logger.h>

#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_TypeBase.h>
#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_ValidatorResult.h>
#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_PrimitiveTypeFlags.h>
#include <DALHAL/Core/JsonConfig/Types/Root/DALHAL_JSON_Schema_EmptyPolicy.h>
#include <DALHAL/Core/JsonConfig/Types/Structures/DALHAL_JSON_Schema_ArrayBase.h>

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_TypesRegistry.h>

namespace DALHAL {

    namespace JsonSchema {

        /** 
         * FieldArrayPrimitive represents a JSON array where each element is a native JSON type:
         * Number (int, float), Bool, or String.
         * Used for unstructured arrays
         */
        struct SchemaArrayOfPrimitives : SchemaArrayBase {
            
            static const FieldTypeRegistryDefine RegistryDefine;
        protected:
            static void ValidateSchema(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, bool& anyError);
            static ValidatorResult ValidateJson(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, const JsonVariant& jsonObj, bool& anyError);
            static void SchemaToJson(const SchemaTypeBase& fieldSchema, std::string& out);
            static const char* GetJavaScriptValidator();

        public:
            bool ExtractValues(const JsonVariant& jsonObj, HALValue** outValues, int& valueCount) const;
            uint8_t primitiveTypeFlags;

            // Number-only array
            constexpr SchemaArrayOfPrimitives(const char* name, FieldPolicy policy, uint8_t primitiveTypeFlags = PrimitiveTypeFlags::AllowAll, EmptyPolicy emptyPolicy = EmptyPolicy::Warn)
                : SchemaArrayBase(name, FieldType::ArrayOfPrimitives, policy, emptyPolicy), primitiveTypeFlags(primitiveTypeFlags)
            {}
        };

    }

}