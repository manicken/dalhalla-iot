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

#include "DALHAL_ScriptArray_JSON_Schema.h"

#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_TypeBase.h>
#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_PrimitiveTypeFlags.h>
#include <DALHAL/Core/JsonConfig/Types/Primitives/DALHAL_JSON_Schema_Bool.h>
#include <DALHAL/Core/JsonConfig/Types/Structures/DALHAL_JSON_Schema_ArrayOfPrimitives.h>
#include <DALHAL/Core/JsonConfig/Types/Root/DALHAL_JSON_Schema_JsonObjectSchema.h>

#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Base.h>

#include "DALHAL_ScriptArray.h"

namespace DALHAL {

    namespace JsonSchema {

        namespace ScriptArray {

            constexpr SchemaBool readonlyField = {"readonly", FieldPolicy::Optional, false};

            constexpr SchemaArrayOfPrimitives scriptArrayItems = {"items", FieldPolicy::Required, PrimitiveTypeFlags::AllowNumbers, EmptyPolicy::Error};

            constexpr const SchemaTypeBase* fields[] = {
                &CommonBase::disabled_type_uidreq_note_group, // DALHAL_CommonSchemas_Base
                &readonlyField,
                &scriptArrayItems,
                nullptr,
            };

            constexpr JsonObjectSchema Root = {
                "ScriptArray",
                fields,
                nullptr, // no modes
                nullptr,  // no constraints
                EmptyPolicy::Warn,
                UnknownFieldPolicy::Warn,
            };

            void Extractors::Apply(DALHAL::DeviceCreateContext& context, DALHAL::ScriptArray* out) {
                const JsonVariant& jsonObj = *(context.jsonObjItem);
                out->uid = encodeUID(JsonSchema::CommonBase::uidFieldRequired.ExtractFrom(jsonObj));
                out->readOnly = JsonSchema::ScriptArray::readonlyField.ExtractFrom(jsonObj);
                out->values = nullptr; // allways set it to nullptr
                if (JsonSchema::ScriptArray::scriptArrayItems.ExtractValues(jsonObj, &out->values, out->valueCount) == false) {
                    // should never happend
                    GlobalLogger.Error(F("Failed to extract script array items"));
                }
            }

        }

    }

}