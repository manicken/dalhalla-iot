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

#include "DALHAL_REGO600_Register_JSON_Scheme.h"

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_Types.h>
#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_BaseTypes.h>
#include <DALHAL/Core/Manager/DALHAL_GPIO_Manager.h>
#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>

#include <DALHAL/Core/JsonConfig/CommonSchemes/DALHAL_CommonSchemes_Base.h>
#include <DALHAL/Core/JsonConfig/CommonSchemes/DALHAL_CommonSchemes_Pins.h>

namespace DALHAL {

    namespace JsonSchema {

        constexpr const char* registerNames[] = {"GT1", "GT2", "GT3", "GT4", "GT5", "GT6", "GT8", "GT9", "GT10", "GT11", 
                                       "GT3X", 
                                       "P3", "COMP", "EL3", "EL6", "P1", "P2", "VXV", "ALARM", nullptr};

        constexpr FieldString regnameField = {"regname", FieldFlag::Required, "gt1", registerNames, FieldString::AllowedValuesPolicy::IgnoreCase};

        constexpr const FieldBase* fields[] = {
            &uidFieldRequired,  // DALHAL_CommonSchemes_Base
            &regnameField,
            nullptr,
        };

        constexpr JsonObjectScheme REGO600_Register = {
            "REGO600_Register",
            fields,
            nullptr, // no modes
            nullptr,  // no constraints
            EmptyPolicy::Warn,
            UnknownFieldPolicy::Warn,
        };

    }

}