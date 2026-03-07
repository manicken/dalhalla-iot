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

#include <DALHAL/Devices/I2C_Master/_DevicesRegistry/DALHAL_I2C_Master_DevicesRegistry.h>

#include <Arduino.h> // Needed for String class

#include <string>
#include <ArduinoJson.h>
#include <Wire.h>

#include <DALHAL/Core/Device/DALHAL_Device.h>

#include <DALHAL/Config/DALHAL_ReactiveConfig.h>
#if USING_REACTIVE(I2C_DEVICE_PCF8574X)
#include "DALHAL_PCF8574x_Reactive.h"
using PCF8574x_DeviceBase = DALHAL::PCF8574x_Reactive;
#else
using PCF8574x_DeviceBase = DALHAL::Device;
#endif

namespace DALHAL {

    class PCF8574x : public PCF8574x_DeviceBase {
    private:
        uint8_t addr = 0;
        TwoWire* wire;
    public:
        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(const JsonVariant &jsonObj, const char* type, TwoWire& wire);
        static bool HasAddress(uint8_t addr);
        static constexpr I2C_DeviceRegistryDefine RegistryDefine = {
            Create,
            VerifyJSON,
            HasAddress
        };

        PCF8574x(const JsonVariant &jsonObj, const char* type, TwoWire& wire);

        HALOperationResult read(HALValue& val) override;
        HALOperationResult write(const HALValue& val) override;

        String ToString() override;
    };
}