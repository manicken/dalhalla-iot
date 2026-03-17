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

#include <Arduino.h> // Needed for String class

#include <string>
#include <ArduinoJson.h>

#include <DALHAL/Core/Device/DALHAL_Device.h>
#include <DALHAL/Core/Types/DALHAL_Registry.h>

#include <DALHAL/Config/DALHAL_ReactiveConfig.h>
#if USING_REACTIVE(ANALOG_INPUT)
#include "DALHAL_AnalogInput_Reactive.h"
using AnalogInput_DeviceBase = DALHAL::AnalogInput_Reactive;
#else
using AnalogInput_DeviceBase = DALHAL::Device;
#endif

namespace DALHAL {

#if defined(ESP32) || defined(_WIN32)
    class AnalogInput : public AnalogInput_DeviceBase {
    public: // public static fields and exposed external structures
        static const Registry::DefineBase RegistryDefine;
        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(DeviceCreateContext& context);
        
    private:
        uint8_t pin = 0;

    public:
        AnalogInput(DeviceCreateContext& context);
        ~AnalogInput();
        void loop() override;
        HALOperationResult read(HALValue &val) override;
        String ToString() override;
    };
#endif
    // TODO implement Analog Input Config to set resolution, but resolution could be individual as it can be set before each read

}