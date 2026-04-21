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

#include <DALHAL/Core/JsonConfig/Types/Structures/DALHAL_JSON_Schema_Object.h>

#include <DALHAL/Core/JsonConfig/Types/Root/DALHAL_JSON_Schema_JsonObjectSchema.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/DALHAL_JSON_Schema_HardwarePin.h>
#include <DALHAL/Core/JsonConfig/Types/Primitives/DALHAL_JSON_Schema_UInt.h>

#define DALHAL_DEVICE_ACTUATOR_CFG_NAME_MIN_END_STOP "MinEndStop"
#define DALHAL_DEVICE_ACTUATOR_CFG_NAME_MAX_END_STOP "MaxEndStop"

namespace DALHAL {

    namespace JsonSchema {

        extern const JsonObjectSchema Actuator;

        extern const SchemaHardwarePin pin_hbridge_a_field;
        extern const SchemaHardwarePin pin_hbridge_b_field;
        extern const SchemaHardwarePin pin_hbridge_open_field;
        extern const SchemaHardwarePin pin_hbridge_close_field;
        extern const SchemaHardwarePin pin_direnable_dir_field;
        extern const SchemaHardwarePin pin_direnable_enable_field;
        // this is a optional field in direnable mode
        extern const SchemaHardwarePin pin_direnable_break_field;

        extern const SchemaObject minEndStopField;
        extern const SchemaObject maxEndStopField;

        extern const SchemaUInt timeout_ms_field;

    }
}