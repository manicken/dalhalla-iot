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

#include "../../Core/DALHAL_Device.h"
#include "../DALHAL_DeviceTypesRegistry.h"

namespace DALHAL {

    // ██████  ██     ██ ███    ███      █████  ███    ██  █████  ██       ██████   ██████      ██     ██ ██████  ██ ████████ ███████      ██████ ███████  ██████  
    // ██   ██ ██     ██ ████  ████     ██   ██ ████   ██ ██   ██ ██      ██    ██ ██           ██     ██ ██   ██ ██    ██    ██          ██      ██      ██       
    // ██████  ██  █  ██ ██ ████ ██     ███████ ██ ██  ██ ███████ ██      ██    ██ ██   ███     ██  █  ██ ██████  ██    ██    █████       ██      █████   ██   ███ 
    // ██      ██ ███ ██ ██  ██  ██     ██   ██ ██  ██ ██ ██   ██ ██      ██    ██ ██    ██     ██ ███ ██ ██   ██ ██    ██    ██          ██      ██      ██    ██ 
    // ██       ███ ███  ██      ██     ██   ██ ██   ████ ██   ██ ███████  ██████   ██████       ███ ███  ██   ██ ██    ██    ███████      ██████ ██       ██████  

    class PWMAnalogWriteConfig  : public Device { // this do include the base class Device mostly so that the loaded devices can be printed for debug 
    public:
        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(const JsonVariant &jsonObj, const char* type);
        static constexpr DeviceRegistryDefine RegistryDefine = {
            UseRootUID::Mandatory,
            Create,
            VerifyJSON
        };
        static uint8_t resolution; // used together with inv_out to get correct value
        static uint32_t frequency;
        
        PWMAnalogWriteConfig(const JsonVariant &jsonObj, const char* type);

        HALOperationResult write(const HALWriteStringRequestValue& value) override;
        String ToString() override;

    };

    // ██████  ██     ██ ███    ███      █████  ███    ██  █████  ██       ██████   ██████      ██     ██ ██████  ██ ████████ ███████ 
    // ██   ██ ██     ██ ████  ████     ██   ██ ████   ██ ██   ██ ██      ██    ██ ██           ██     ██ ██   ██ ██    ██    ██      
    // ██████  ██  █  ██ ██ ████ ██     ███████ ██ ██  ██ ███████ ██      ██    ██ ██   ███     ██  █  ██ ██████  ██    ██    █████   
    // ██      ██ ███ ██ ██  ██  ██     ██   ██ ██  ██ ██ ██   ██ ██      ██    ██ ██    ██     ██ ███ ██ ██   ██ ██    ██    ██      
    // ██       ███ ███  ██      ██     ██   ██ ██   ████ ██   ██ ███████  ██████   ██████       ███ ███  ██   ██ ██    ██    ███████ 

    class PWMAnalogWrite : public Device {
    private:
        uint8_t pin = 0;
        uint8_t inv_out = 0;
        uint32_t value = 0; // for readback only
        uint32_t getInvValue(uint32_t val);
    public:
        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(const JsonVariant &jsonObj, const char* type);
        static constexpr DeviceRegistryDefine RegistryDefine = {
            UseRootUID::Mandatory,
            Create,
            VerifyJSON
        };
        PWMAnalogWrite(const JsonVariant &jsonObj, const char* type);
        ~PWMAnalogWrite();

        HALOperationResult read(HALValue &val) override;
        HALOperationResult write(const HALValue &val) override;
        String ToString() override;
    };

    // TODO implement HW_PWM to use with esp32 only (ledc based)

}