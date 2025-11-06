/*
  Dalhalla IoT — JSON-configured HAL/DAL + Script Engine
  HAL = Hardware Abstraction Layer
  DAL = Device Abstraction Layer

  Provides IoT firmware building blocks for home automation and smart sensors.

  Copyright (c) 2025 Jannik Svensson

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/


#include "HAL_JSON_I2C_BUS_DeviceTypeReg.h"

// Available I2C device types here
#include "../Display_SSD1306/HAL_JSON_Display_SSD1306.h"
#include "../HAL_JSON_PCF8574x.h"

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