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

#include <stdlib.h>

#include <DALHAL/Support/DALHAL_Logger.h>

#include <DALHAL/Core/Manager/DALHAL_GPIO_Manager.h>

#include "DALHAL_JSON_Schema_TypeBase.h"

namespace DALHAL {

    namespace JsonSchema {

        struct SchemaHardwarePin : SchemaTypeBase {
            DALHAL_GPIO_MGR_PINFUNC_TYPE mode;
            constexpr SchemaHardwarePin(const char* name, FieldPolicy policy, DALHAL_GPIO_MGR_PINFUNC_TYPE mode)
                : SchemaTypeBase(name, FieldType::HardwarePin, policy), mode(mode) {} // here pin could use Int but that is not how pins are validated they instead use GPIO_manager for validity
        };

    }

}