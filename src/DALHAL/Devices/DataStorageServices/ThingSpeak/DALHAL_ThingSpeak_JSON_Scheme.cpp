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

#include "DALHAL_ThingSpeak_JSON_Scheme.h"

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_Types.h>
#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_BaseTypes.h>
#include <DALHAL/Core/Manager/DALHAL_GPIO_Manager.h>
//#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>

#include <DALHAL/Core/JsonConfig/CommonSchemes/DALHAL_CommonSchemes_Base.h>
#include <DALHAL/Core/JsonConfig/CommonSchemes/DALHAL_CommonSchemes_Time.h>

namespace DALHAL {

    namespace JsonSchema {

        constexpr FieldUInt firstUpdateAfterSecondsField = {DALHAL_DEVICE_THINGSPEAK_CFG_NAME_FIRST_UPDATE_AFTER_SECONDS, FieldFlag::Optional, 0, 0, 0};
        constexpr FieldBool testserverField = {DALHAL_DEVICE_THINGSPEAK_CFG_NAME_TESTSERVER, FieldFlag::Optional, false};
        constexpr FieldString keyField = {DALHAL_DEVICE_THINGSPEAK_CFG_NAME_KEY, FieldFlag::Required, "0123456789ABCDEF", 16, 16}; // here min/max defines so that the string must be exact 16 characters long

        constexpr FieldString itemsF1 = {"1", FieldType::UID_Path, FieldFlag::Optional, nullptr, 0};
        constexpr FieldString itemsF2 = {"2", FieldType::UID_Path, FieldFlag::Optional, nullptr, 0};
        constexpr FieldString itemsF3 = {"3", FieldType::UID_Path, FieldFlag::Optional, nullptr, 0};
        constexpr FieldString itemsF4 = {"4", FieldType::UID_Path, FieldFlag::Optional, nullptr, 0};
        constexpr FieldString itemsF5 = {"5", FieldType::UID_Path, FieldFlag::Optional, nullptr, 0};
        constexpr FieldString itemsF6 = {"6", FieldType::UID_Path, FieldFlag::Optional, nullptr, 0};
        constexpr FieldString itemsF7 = {"7", FieldType::UID_Path, FieldFlag::Optional, nullptr, 0};
        constexpr FieldString itemsF8 = {"8", FieldType::UID_Path, FieldFlag::Optional, nullptr, 0};

        constexpr const FieldBase* itemsFields[] = {&itemsF1, &itemsF2, &itemsF3, &itemsF4, &itemsF5, &itemsF6, &itemsF7, &itemsF8, nullptr};

        constexpr JsonObjectScheme itemsFieldScheme = {
            "items",
            itemsFields,
            nullptr, // no modes
            nullptr,  // no constraints
            EmptyPolicy::Error,
            UnknownFieldPolicy::Error,
        };

        constexpr FieldObject itemsField = {"items", FieldFlag::Required, &itemsFieldScheme};

        constexpr const FieldBase* fields[] = {
            &typeField,         // DALHAL_CommonSchemes_Base
            &uidFieldRequired,  // DALHAL_CommonSchemes_Base
            &refreshTimeGroupFields,
            &itemsField,
            nullptr,
        };

        constexpr JsonObjectScheme ThingSpeak = {
            "ThingSpeak",
            fields,
            nullptr, // no modes
            nullptr, // no constraints
            EmptyPolicy::Warn,
            UnknownFieldPolicy::Warn,
        };

    }

}