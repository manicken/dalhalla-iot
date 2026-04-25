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
#include <ArduinoJson.h>
#include <Wire.h>

#include <DALHAL/Core/Device/DALHAL_Device.h>
#include <DALHAL/Core/Types/DALHAL_Registry.h>

#include <DALHAL/Core/Reactive/DALHAL_ReactiveEvent.h>
#include <DALHAL/Config/DALHAL_ReactiveConfig.h>
#if USING_REACTIVE(I2C_MASTER)
#include "DALHAL_I2C_Master_Reactive.h"
using I2C_Master_DeviceBase = DALHAL::I2C_Master_Reactive;
#else
using I2C_Master_DeviceBase = DALHAL::Device;
#endif

namespace DALHAL {

    namespace JsonSchema { namespace I2C_Master { struct Extractors; } } // forward declaration

    class I2C_Master : public I2C_Master_DeviceBase {
        friend struct JsonSchema::I2C_Master::Extractors; // allow access to private memebers of this class from the schema extractor

    public: // public static fields and exposed external structures
        static const Registry::DefineBase RegistryDefine;
        static Device* Create(DeviceCreateContext& context);

    private:
        TwoWire* wire = nullptr;
        Device** devices = nullptr;
        int deviceCount = 0;

        uint8_t sckpin = 0;
        uint8_t sdapin = 0;
        uint32_t freq = 0;

    public:
        I2C_Master(DeviceCreateContext& context);
        ~I2C_Master() override;

        void loop() override;

        DeviceFindResult findDevice(UIDPath& path, Device*& outDevice) override;

        HALOperationResult read(const HALReadStringRequestValue& val) override;
        HALOperationResult write(const HALWriteStringRequestValue& val) override;

        String ToString() override;
    };
    
}