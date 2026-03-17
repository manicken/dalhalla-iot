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

#include "DALHAL_OneWireTempAutoRefresh.h"

#include <Arduino.h> // Needed for String class

#include <OneWire.h>
#include <DallasTemperature.h>

#include <ArduinoJson.h>

#include <DALHAL/Core/Device/DALHAL_Device.h>
#include <DALHAL/Core/Types/DALHAL_Registry.h>

#define DALHAL_KEYNAME_ONE_WIRE_ROMID       "romid"
#define DALHAL_KEYNAME_ONE_WIRE_TEMPFORMAT "format"

#include <DALHAL/Config/DALHAL_ReactiveConfig.h>
#if USING_REACTIVE(ONE_WIRE_TEMP_DEVICE)
#include "DALHAL_OneWireTempDevice_Reactive.h"
using OneWireTempDevice_DeviceBase = DALHAL::OneWireTempDevice_Reactive;
#else
using OneWireTempDevice_DeviceBase = DALHAL::Device;
#endif

#if HAS_REACTIVE_VALUE_CHANGE(ONE_WIRE_TEMP_DEVICE)
#include <DALHAL/Core/Types/DALHAL_ValueReactive.h>
using OneWireTempDevice_ValueBase = DALHAL::ReactiveHALValue;
#else
using OneWireTempDevice_ValueBase = DALHAL::HALValue;
#endif

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

    class OneWireTempDevice : public OneWireTempDevice_DeviceBase {
    public: // public static fields and exposed external structures
        static bool VerifyJSON(const JsonVariant &jsonObj);
    
    public:
        OneWireAddress romid;
        OneWireTempDeviceTempFormat format = OneWireTempDeviceTempFormat::Celsius;
        OneWireTempDevice_ValueBase value = 0.0f;
    
    public:
        OneWireTempDevice(DeviceCreateContext& context);
        ~OneWireTempDevice();
        
        HALOperationResult read(HALValue& val) override;

        void read(DallasTemperature& dTemp);

        String ToString() override;
    };

    class OneWireTempDeviceAtRoot : public OneWireTempDevice {
    public: // public static fields and exposed external structures
        static const Registry::DefineRoot RegistryDefine;
        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(DeviceCreateContext& context);

    private:
        OneWireTempAutoRefresh autoRefresh;
        uint8_t pin;
        OneWire* oneWire = nullptr;
        DallasTemperature* dTemp = nullptr;
        void requestTemperatures();
        void readAll();

    public:
        OneWireTempDeviceAtRoot(DeviceCreateContext& context);
        ~OneWireTempDeviceAtRoot();
#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
        HALOperationResult write(const HALValue& val) override;
#endif
        void loop() override;

        String ToString() override;
    };
}