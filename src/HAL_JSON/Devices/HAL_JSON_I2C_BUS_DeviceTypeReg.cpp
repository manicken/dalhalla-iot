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

#include "HAL_JSON_I2C_BUS_DeviceTypeReg.h"

// Available I2C device types here
#include "Display_SSD1306/HAL_JSON_Display_SSD1306.h"
#include "HAL_JSON_PCF8574x.h"

namespace HAL_JSON {

    const I2C_DeviceTypeDef I2C_DeviceRegistry[] = {
        {"SSD1306", Display_SSD1306::Create, Display_SSD1306::VerifyJSON, Display_SSD1306::HasAddress},
        {"PCF8574x",  PCF8574x::Create, PCF8574x::VerifyJSON, PCF8574x::HasAddress},
        /** mandatory null terminator */
        {nullptr, nullptr, nullptr}
    };
    const I2C_DeviceTypeDef* GetI2C_DeviceTypeDef(const char* type) {
        int i=0;
        while (true) {
            const I2C_DeviceTypeDef& def = I2C_DeviceRegistry[i++];
            if (def.typeName == nullptr) break;
            if (strcasecmp(def.typeName, type) == 0) return &def;
        }
        return nullptr;
    }
    std::string describeI2CAddress(uint8_t addr) {
        std::string deviceNames;
        deviceNames.reserve(32); // to minimize heap fragmentation
        bool first = true;
        for (int i = 0; I2C_DeviceRegistry[i].typeName != nullptr; i++) {
            if (I2C_DeviceRegistry[i].HasAddress_Function(addr)) {
                if (first == false) deviceNames += ',';
                else first = false;
                deviceNames += I2C_DeviceRegistry[i].typeName;
            }
        }
        return std::move(deviceNames);
    }
}