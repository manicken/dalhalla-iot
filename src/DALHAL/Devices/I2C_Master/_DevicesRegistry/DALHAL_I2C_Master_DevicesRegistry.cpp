/*
  Dalhalla IoT — JSON-configured HAL/DAL + Script Engine
  HAL = Hardware Abstraction Layer
  DAL = Device Abstraction Layer

  Provides IoT firmware building blocks for home automation and smart sensors.

  Copyright (C) 2026 Jannik Svensson

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


#include "DALHAL_I2C_Master_DevicesRegistry.h"

// Available I2C device types here
#include <DALHAL/Devices/Display/SSD1306/DALHAL_Display_SSD1306.h>
#include <DALHAL/Devices/I2C_Master/Devices/PCF8574x/DALHAL_PCF8574x.h>



namespace DALHAL {

    constexpr Registry::Item items[] = {
        {"SSD1306",   &Display_SSD1306::RegistryDefine},
        {"PCF8574x",  &PCF8574x::RegistryDefine},
    };

    constexpr Registry::DeviceRegistry I2C_DeviceRegistry = {items, sizeof(items)/sizeof(items[0]), "I2C Master", "ROOT.I2C_MASTER"};

    void describeI2CAddress(uint8_t addr, StringBuilderStreamer& sbs) {
        bool first = true;
        for (size_t i = 0; i < I2C_DeviceRegistry.count; i++) {
            const I2C_RegistryDefine* i2c_def = static_cast<const I2C_RegistryDefine*>(I2C_DeviceRegistry.items[i].def);
            if (i2c_def->HasAddress_Function(addr)) {
                if (first == false) sbs.write(',');
                else first = false;
                sbs.write(I2C_DeviceRegistry.items[i].typeName);
            }
        }
    }
}