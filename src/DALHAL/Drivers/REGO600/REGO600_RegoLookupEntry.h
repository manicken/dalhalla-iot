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

#include "stdint.h"
#include "REGO600_CommandID.h"
#include "REGO600_RequestType.h"
#include "WString.h"

namespace Drivers {
    namespace REGO600 {

        union ValueLimit {
            uint16_t u16;
            int16_t  s16;
        };

        enum class ValueType {
            Unset,
            Bool,
            Unsigned,
            Signed,
            Float
        };

        struct RegoLookupEntry {
            PGM_P name;
            uint16_t address;
            //REGO600::OpCodes opcode; this is determined by if it's read/write and set elsewhere
            ValueLimit minVal;      // including minimum
            ValueLimit maxVal;      // including maximum
            ValueType valueType;       
            float multiplier;    //  for example 0.1 @ temperatures
        };

    }
}