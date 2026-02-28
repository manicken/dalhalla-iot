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


#include <OneWire.h>
#include <DallasTemperature.h>

#include <ArduinoJson.h>

#include "../../../Core/Device/DALHAL_Device.h"
#include "../../DeviceRegistry/DALHAL_DeviceTypesRegistry.h"
#include "../../../Core/Reactive/DALHAL_SimpleEventDevice.h"

#include "DALHAL_OneWireTempAutoRefresh.h"


#define DALHAL_KEYNAME_ONE_WIRE_ROMID       "romid"
#define DALHAL_KEYNAME_ONE_WIRE_TEMPFORMAT "format"

namespace DALHAL {

    enum class OneWireTempDeviceTempFormat {
        Celsius,
        Fahrenheit
    };

    typedef struct {
        union {
            uint8_t bytes[8];
            uint64_t id;
        };
    } OneWireAddress;

    class OneWireTempDevice : public SimpleEventDevice {
        
    public:
        OneWireAddress romid;
        OneWireTempDeviceTempFormat format = OneWireTempDeviceTempFormat::Celsius;
        float value = 0.0f;

        static bool VerifyJSON(const JsonVariant &jsonObj);
        
        OneWireTempDevice(const JsonVariant &jsonObj, const char* type);
        ~OneWireTempDevice();
        
        HALOperationResult read(HALValue& val) override;

        void read(DallasTemperature& dTemp);

        String ToString() override;
    };

    class OneWireTempDeviceAtRoot : public OneWireTempDevice {
    private:
        OneWireTempAutoRefresh autoRefresh;
        uint8_t pin;
        OneWire* oneWire = nullptr;
        DallasTemperature* dTemp = nullptr;
        void requestTemperatures();
        void readAll();
    public:
        
        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(const JsonVariant &jsonObj, const char* type);
        static constexpr DeviceRegistryDefine RegistryDefine = {
            UseRootUID::Mandatory,
            Create,
            VerifyJSON
        };
        OneWireTempDeviceAtRoot(const JsonVariant &jsonObj, const char* type);
        ~OneWireTempDeviceAtRoot();
//#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
        HALOperationResult write(const HALValue& val) override;
//#endif
        void loop() override;

        String ToString() override;
    };
}