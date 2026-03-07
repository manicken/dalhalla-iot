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
#include <Wire.h>

#include <DALHAL/Core/Device/DALHAL_Device.h>
#include <DALHAL/Devices/_Registry/DALHAL_DevicesRegistry.h>

#include <DALHAL/Core/Reactive/DALHAL_ReactiveEvent.h>
#include <DALHAL/Config/DALHAL_ReactiveConfig.h>
#if USING_REACTIVE(_TEMPLATE_)
#include "DALHAL_I2C_Master_Reactive.h"
using I2C_Master_DeviceBase = DALHAL::I2C_Master_Reactive;
#else
using I2C_Master_DeviceBase = DALHAL::Device;
#endif

namespace DALHAL {


    class I2C_Master : public I2C_Master_DeviceBase {
    private:
        uint8_t sckpin = 0;
        uint8_t sdapin = 0;
        uint32_t freq = 0;

        Device** devices;
        int deviceCount;

        TwoWire* wire;

    public:
        
        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(const JsonVariant &jsonObj, const char* type);
        static constexpr DeviceRegistryDefine RegistryDefine = {
            UseRootUID::Mandatory,
            I2C_Master::Create,
            I2C_Master::VerifyJSON
        };
        I2C_Master(const JsonVariant &jsonObj, const char* type);
        ~I2C_Master();

        DeviceFindResult findDevice(UIDPath& path, Device*& outDevice) override;
        void loop() override;

        HALOperationResult read(const HALReadStringRequestValue& val) override;
        HALOperationResult write(const HALWriteStringRequestValue& val) override;

        String ToString() override;
    };

    
}