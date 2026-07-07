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

namespace Drivers {
    namespace REGO600 {

        enum class CommandID {
            ReadFrontPanel,
            WriteFrontPanel,
            ReadSystemRegister,
            WriteSystemRegister,
            ReadTimerRegisters,
            WriteTimerRegisters,
            ReadRegister_1B08,
            WriteRegister_1B08,
            ReadDisplay,
            RamDump,
            ReadLastError,
            ReadPrevError,
            ReadRegoVersion,
            NotSet,
        };
        
    }
}