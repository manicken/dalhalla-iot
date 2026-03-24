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

#include "DALHAL_TX433_Unit_TypeLC_JSON_Scheme.h"

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_Types.h>
#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_BaseTypes.h>
#include <DALHAL/Core/Manager/DALHAL_GPIO_Manager.h>
#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>

#include <DALHAL/Core/JsonConfig/CommonSchemes/DALHAL_CommonSchemes_Base.h>

namespace DALHAL {

    namespace JsonSchema {

        constexpr FieldString anidField = {"anid", FieldFlag::ModeDefine, "Id01", 4, 4};
        constexpr FieldString hexidField = {"hexid", FieldFlag::ModeDefine, "090A0B", 6, 6};

        constexpr FieldUInt grp_btnField = {"grp_btn", FieldFlag::Optional, 0, 3, 0};
        constexpr FieldUInt btnField = {"btn", FieldFlag::Optional, 0, 3, 0};
        constexpr FieldUInt stateField = {"state", FieldFlag::Optional, 0, 1, 0};

        constexpr ModeConjunctionDefine conjunctions_anid_Mode[] = {
            { &anidField, true },  // group must exist for this mode
            { &hexidField, false }, // group must NOT exist for this mode
            { nullptr, false}
        };

        constexpr ModeConjunctionDefine conjunctions_hexid_Mode[] = {
            { &anidField, false },  // group must NOT exist for this mode
            { &hexidField, true }, // group must exist for this mode
            { nullptr, false}
        };

        constexpr const ModeSelector modes[] = {
            {"AlphaNumeric ID mode", conjunctions_anid_Mode},
            {"hex ID mode", conjunctions_hexid_Mode},
            {nullptr, nullptr}
        };

        
        constexpr const FieldBase* fields[] = {
            &typeField,         // DALHAL_CommonSchemes_Base
            &uidFieldRequired,  // DALHAL_CommonSchemes_Base
            &anidField,
            &hexidField,
            &grp_btnField,
            &btnField,
            &stateField,
        };

        constexpr JsonObjectScheme TX433_Unit_TypeLC = {
            "TX433_Unit_TypeLC",
            fields,
            modes, // no modes
            nullptr,  // no constraints
            EmptyPolicy::Warn,
            UnknownFieldPolicy::Warn,
        };

    }

}