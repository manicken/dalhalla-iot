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

#include "../../Core/Device/DALHAL_Device.h"
#include "../DALHAL_DeviceTypesRegistry.h"

#include <Wire.h>

namespace DALHAL {


    class I2C_BUS : public Device {
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
            Create,
            VerifyJSON
        };
        I2C_BUS(const JsonVariant &jsonObj, const char* type);
        ~I2C_BUS();

        DeviceFindResult findDevice(UIDPath& path, Device*& outDevice) override;
        void loop() override;

        HALOperationResult read(const HALReadStringRequestValue& val) override;
        HALOperationResult write(const HALWriteStringRequestValue& val) override;

        String ToString() override;
    };

    
}