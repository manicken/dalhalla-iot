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

#include "DALHAL_SinglePulseOutput_JSON_Schema.h"

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_Types.h>
#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_BaseTypes.h>
#include <DALHAL/Core/Manager/DALHAL_GPIO_Manager.h>

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h> // DALHAL_KEYNAME_SINGLE_PULSE_OUTPUT_DEFAULT_PULSE_LENGHT

#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Base.h>
#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Pins.h>

namespace DALHAL {

    namespace JsonSchema {

        constexpr FieldHardwarePin pinField = { DALHAL_COMMON_CFG_NAME_PIN, FieldPolicy::Required, static_cast<uint8_t>(GPIO_manager::PinFunc::IN) };
        constexpr FieldUInt pulseLengthField = { DALHAL_KEYNAME_SINGLE_PULSE_OUTPUT_DEFAULT_PULSE_LENGHT, FieldPolicy::Optional, 1, 0, 500};
        
        constexpr FieldString activeLevelField = { "activeLevel", FieldPolicy::Optional, DALHAL_COMMON_CFG_VALUE_PIN_LEVEL_HIGH, activeLevelStrings, FieldString::AllowedValuesPolicy::IgnoreCase};


        constexpr const FieldBase* fields[] = {
            &typeField,         // DALHAL_CommonSchemas_Base
            &uidFieldRequired,  // DALHAL_CommonSchemas_Base
            &pinField,
            &pulseLengthField,
            &activeLevelField,
            nullptr,
        };

        constexpr JsonObjectSchema SinglePulseOutput = {
            "SinglePulseOutput",
            fields,
            nullptr, // no modes
            nullptr,  // no constraints
            EmptyPolicy::Warn,
            UnknownFieldPolicy::Warn,
        };

    }

}