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

#include "DALHAL_ThingSpeak_JSON_Schema.h"

#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_TypeBase.h>
#include <DALHAL/Core/JsonConfig/Types/Structures/DALHAL_JSON_Schema_Object.h>
#include <DALHAL/Core/JsonConfig/Types/Primitives/DALHAL_JSON_Schema_UInt.h>
#include <DALHAL/Core/JsonConfig/Types/Primitives/DALHAL_JSON_Schema_Bool.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/String/DALHAL_JSON_Schema_StringSizeConstrained.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/String/DALHAL_JSON_Schema_StringUID_Path.h>
#include <DALHAL/Core/JsonConfig/Types/Root/DALHAL_JSON_Schema_JsonObjectSchema.h>

#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Base.h>
#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Time.h>

namespace DALHAL {

    namespace JsonSchema {

        namespace ThingSpeak {

            constexpr SchemaUInt firstUpdateAfterSecondsField = {"firstUpdateAfterSeconds", FieldPolicy::Optional, (uint)0, (uint)0, (uint)0};
            constexpr SchemaBool testserverField = {"testserver", FieldPolicy::Optional, false};
            constexpr SchemaStringSizeConstrained keyField = {"key", FieldPolicy::Required, "0123456789ABCDEF", (uint)16, (uint)16}; // here min/max defines so that the string must be exact 16 characters long

            constexpr SchemaStringUID_Path itemsF1 = {"1", FieldPolicy::Optional};
            constexpr SchemaStringUID_Path itemsF2 = {"2", FieldPolicy::Optional};
            constexpr SchemaStringUID_Path itemsF3 = {"3", FieldPolicy::Optional};
            constexpr SchemaStringUID_Path itemsF4 = {"4", FieldPolicy::Optional};
            constexpr SchemaStringUID_Path itemsF5 = {"5", FieldPolicy::Optional};
            constexpr SchemaStringUID_Path itemsF6 = {"6", FieldPolicy::Optional};
            constexpr SchemaStringUID_Path itemsF7 = {"7", FieldPolicy::Optional};
            constexpr SchemaStringUID_Path itemsF8 = {"8", FieldPolicy::Optional};

            constexpr const SchemaTypeBase* itemsFields[] = {&itemsF1, &itemsF2, &itemsF3, &itemsF4, &itemsF5, &itemsF6, &itemsF7, &itemsF8, nullptr};

            constexpr JsonObjectSchema itemsFieldScheme = {
                "ThingSpeakField",
                itemsFields,
                nullptr, // no modes
                nullptr,  // no constraints
                EmptyPolicy::Error,
                UnknownFieldPolicy::Error,
            };

            constexpr SchemaObject itemsField = {"items", FieldPolicy::Required, &itemsFieldScheme};

            constexpr const SchemaTypeBase* fields[] = {
                &CommonBase::disabled_type_uidreq_note_group, // DALHAL_CommonSchemas_Base
                &CommonTime::refreshTimeGroupFields,
                &firstUpdateAfterSecondsField,
                &testserverField,
                &keyField,
                &itemsField,
                nullptr,
            };

            constexpr JsonObjectSchema Root = {
                "ThingSpeak",
                fields,
                nullptr, // no modes
                nullptr, // no constraints
                EmptyPolicy::Warn,
                UnknownFieldPolicy::Warn,
            };

        }

    }

}