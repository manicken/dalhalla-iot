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

#include "DALHAL_TX433_Unit_TypeLC_JSON_Schema.h"

#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_TypeBase.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_StringSizeConstrained.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_UInt.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_JsonObjectSchema.h>

#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Base.h>

namespace DALHAL {

    namespace JsonSchema {

        constexpr SchemaStringSizeConstrained anidField = {"anid", FieldPolicy::ModeDefine, "Id01", 4, 4};
        constexpr SchemaStringSizeConstrained hexidField = {"hexid", FieldPolicy::ModeDefine, "090A0B", 6, 6};

        constexpr SchemaUInt grp_btnField = {"grp_btn", FieldPolicy::Optional, 0, 3, 0};
        constexpr SchemaUInt btnField = {"btn", FieldPolicy::Optional, 0, 3, 0};
        constexpr SchemaUInt stateField = {"state", FieldPolicy::Optional, 0, 1, 0};

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

        
        constexpr const SchemaTypeBase* fields[] = {
            &disabled_type_uidreq_note_group, // DALHAL_CommonSchemas_Base
            &anidField,
            &hexidField,
            &grp_btnField,
            &btnField,
            &stateField,
            nullptr,
        };

        constexpr JsonObjectSchema TX433_Unit_TypeLC = {
            "TX433_Unit_TypeLC",
            fields,
            modes, // no modes
            nullptr,  // no constraints
            EmptyPolicy::Warn,
            UnknownFieldPolicy::Warn,
        };

    }

}