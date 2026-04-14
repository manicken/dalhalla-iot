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
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_Object.h>
#include <DALHAL/Core/JsonConfig/Types/Primitives/DALHAL_JSON_Schema_UInt.h>
#include <DALHAL/Core/JsonConfig/Types/Primitives/DALHAL_JSON_Schema_Bool.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_StringSizeConstrained.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_StringUID_Path.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_JsonObjectSchema.h>

#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Base.h>
#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Time.h>

namespace DALHAL {

    namespace JsonSchema {

        constexpr SchemaUInt firstUpdateAfterSecondsField = {DALHAL_DEVICE_THINGSPEAK_CFG_NAME_FIRST_UPDATE_AFTER_SECONDS, FieldPolicy::Optional, 0, 0, 0};
        constexpr SchemaBool testserverField = {DALHAL_DEVICE_THINGSPEAK_CFG_NAME_TESTSERVER, FieldPolicy::Optional, false};
        constexpr SchemaStringSizeConstrained keyField = {DALHAL_DEVICE_THINGSPEAK_CFG_NAME_KEY, FieldPolicy::Required, "0123456789ABCDEF", 16, 16}; // here min/max defines so that the string must be exact 16 characters long

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
            "items",
            itemsFields,
            nullptr, // no modes
            nullptr,  // no constraints
            EmptyPolicy::Error,
            UnknownFieldPolicy::Error,
        };

        constexpr SchemaObject itemsField = {"items", FieldPolicy::Required, &itemsFieldScheme};

        constexpr const SchemaTypeBase* fields[] = {
            &disabled_type_uidreq_note_group, // DALHAL_CommonSchemas_Base
            &refreshTimeGroupFields,
            &firstUpdateAfterSecondsField,
            &testserverField,
            &keyField,
            &itemsField,
            nullptr,
        };

        constexpr JsonObjectSchema ThingSpeak = {
            "ThingSpeak",
            fields,
            nullptr, // no modes
            nullptr, // no constraints
            EmptyPolicy::Warn,
            UnknownFieldPolicy::Warn,
        };

    }

}