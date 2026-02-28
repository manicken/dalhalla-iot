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


#include "DALHAL_I2C_BUS_DeviceTypeReg.h"

// Available I2C device types here
#include "../Display/SSD1306/DALHAL_Display_SSD1306.h"
#include "DALHAL_PCF8574x.h"

namespace DALHAL {

    constexpr I2C_DeviceRegistryDefine RegistryItemNullDefault = {nullptr, nullptr, nullptr};
    constexpr I2C_DeviceRegistryItem RegistryTerminatorItem = {nullptr, RegistryItemNullDefault};

    constexpr I2C_DeviceRegistryItem I2C_DeviceRegistry[] = {
        {"SSD1306",   Display_SSD1306::RegistryDefine},
        {"PCF8574x",  PCF8574x::RegistryDefine},
        /** mandatory null terminator */
        RegistryTerminatorItem
    };
    const I2C_DeviceRegistryItem& GetI2C_DeviceTypeDef(const char* type) {
        int i=0;
        while (true) {
            const I2C_DeviceRegistryItem& regItem = I2C_DeviceRegistry[i++];
            if (regItem.typeName == nullptr) break;
            if (strcasecmp(regItem.typeName, type) == 0) return regItem;
        }
        return RegistryTerminatorItem;
    }
    std::string describeI2CAddress(uint8_t addr) {
        std::string deviceNames;
        deviceNames.reserve(32); // to minimize heap fragmentation
        bool first = true;
        for (int i = 0; I2C_DeviceRegistry[i].typeName != nullptr; i++) {
            if (I2C_DeviceRegistry[i].def.HasAddress_Function(addr)) {
                if (first == false) deviceNames += ',';
                else first = false;
                deviceNames += I2C_DeviceRegistry[i].typeName;
            }
        }
        return std::move(deviceNames);
    }
}