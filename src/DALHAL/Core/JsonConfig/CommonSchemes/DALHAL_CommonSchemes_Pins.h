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

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_Types.h>

#define DALHAL_COMMON_CFG_NAME_PIN "pin"
#define DALHAL_COMMON_CFG_NAME_PIN_ACTIVE_HIGH "ActiveHigh"
#define DALHAL_COMMON_CFG_VALUE_PIN_LEVEL_LOW "low"
#define DALHAL_COMMON_CFG_VALUE_PIN_LEVEL_HIGH "high"

namespace DALHAL {

    namespace JsonSchema {
        extern const FieldHardwarePin InputPinField;
        extern const FieldHardwarePin OutputPinField;
        extern const FieldHardwarePin InputOutputPinField;

        extern const JsonObjectScheme InputPinScheme;
        extern const JsonObjectScheme OutputPinScheme;

        static constexpr const char* activeLevelStrings[] = {DALHAL_COMMON_CFG_VALUE_PIN_LEVEL_LOW, DALHAL_COMMON_CFG_VALUE_PIN_LEVEL_HIGH, nullptr};

    }
    
}