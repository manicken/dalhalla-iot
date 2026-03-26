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

#include "DALHAL_REGO600_Register_JSON_Schema.h"

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_Types.h>
#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_BaseTypes.h>

#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Base.h>
#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Pins.h>
#include <DALHAL/Drivers/REGO600.h>

namespace DALHAL {

    namespace JsonSchema {

        constexpr FieldStringConstraint regnameField = {"regname", FieldPolicy::Required, Drivers::REGO600::SystemRegisterTable_ItemExists, Drivers::REGO600::SystemRegisterTable_GetAllNamesAsJsonStringArray};

        constexpr const FieldBase* fields[] = {
            &disabled_uidreq_note_group, // DALHAL_CommonSchemas_Base
            &regnameField,
            nullptr,
        };

        constexpr JsonObjectSchema REGO600_Register = {
            "REGO600_Register",
            fields,
            nullptr, // no modes
            nullptr,  // no constraints
            EmptyPolicy::Warn,
            UnknownFieldPolicy::Warn,
        };

    }

}