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

#include "DALHAL_JSON_Schema.h"

#include <ArduinoJSON.h>
using json = JsonVariant;

namespace DALHAL {

    namespace JsonSchema {

        bool validateField(const json& j, const DALHAL::JsonSchema::FieldBase* field, std::string& error);
        bool validateAnyOfGroup(const json& j, const DALHAL::JsonSchema::AnyOfGroup* group, std::string& error);
        bool validateMode(const json& j, const DALHAL::JsonSchema::ModeSelector* mode, std::string& error);
        bool validateDevice(const json& j, const DALHAL::JsonSchema::Device* device, uint32_t modeId, std::string& error);

    }
    
}