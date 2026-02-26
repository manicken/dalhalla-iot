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

#include "DALHAL_SerialAPI.h"
#include "../Core/Types/DALHAL_ZeroCopyString.h"
#include "DALHAL_CommandExecutor.h"

#include <Arduino.h> // TODO remove dependency

namespace DALHAL {

    /*static*/ void SerialAPI::loop() {
        if (Serial.available()) {
            String cmd = Serial.readStringUntil('\n');
            cmd.trim();
            ZeroCopyString zcStrCmd(cmd.c_str());

            CommandExecutor::execute(zcStrCmd, [](const std::string& msg){ 
                printf("%s\r\n", msg.c_str());
            });
        }
    }
    
}