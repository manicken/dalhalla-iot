/*
  Dalhalla IoT — JSON-configured HAL/DAL + Script Engine
  HAL = Hardware Abstraction Layer
  DAL = Device Abstraction Layer

  Provides IoT firmware building blocks for home automation and smart sensors.

  Copyright (C) 2025 Jannik Svensson

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


// for raw h-bridge control using forward and backward pins
#define DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_PIN_A        "pinA"
#define DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_PIN_B        "pinB"
#define DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_PIN_SET     "pinSet"
#define DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_PIN_RESET    "pinRst"
// for dir/enable pin mode
#define DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_PIN_DIR      "pinDir"
#define DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_PIN_ENABLE   "pinEnable"

// 
#define DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_RESET_STATE "ResetState"
#define DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_SET_STATE "SetState"

#define DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_TIMEOUT_MS "timeoutMs"
#define DALHAL_DEVICE_LATCHING_RELAY_CFG_DEFAULT_TIMEOUT_MS 500

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_BaseTypes.h>
namespace DALHAL {

    namespace JsonSchema {

        extern const JsonObjectScheme LatchingRelayDevice;

    }
}