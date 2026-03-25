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

#include <Arduino.h> // Needed for String class

#include <string>


#include "DALHAL_Display_SSD1306_Element.h"

// Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DALHAL/Devices/I2C_Master/_DevicesRegistry/DALHAL_I2C_Master_DevicesRegistry.h>
#include <DALHAL/Core/Types/DALHAL_Registry.h>

#include <DALHAL/Config/DALHAL_ReactiveConfig.h>
#if USING_REACTIVE(_TEMPLATE_)
#include "DALHAL_Display_SSD1306_Reactive.h"
using Display_SSD1306_DeviceBase = DALHAL::Display_SSD1306_Reactive;
#else
using Display_SSD1306_DeviceBase = DALHAL::Device;
#endif

namespace DALHAL {

    class Display_SSD1306 : public Display_SSD1306_DeviceBase {
    public: // public static fields and exposed external structures
        static const I2C_RegistryDefine RegistryDefine;
        static Device* Create(DeviceCreateContext& context);
        static bool HasAddress(uint8_t addr);

    private:
        Adafruit_SSD1306* display;

        Device** elements;
        int elementCount;

    public:        
        Display_SSD1306(I2C_Master_CreateFunctionContext& context);
        ~Display_SSD1306();

        HALOperationResult write(const HALWriteStringRequestValue& val) override;
        DeviceFindResult findDevice(UIDPath& path, Device*& outDevice) override;
        void loop() override;

        String ToString() override;
    };
}