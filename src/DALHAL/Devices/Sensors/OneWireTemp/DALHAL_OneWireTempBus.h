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

#include "DALHAL_OneWireTempDevice.h"
#include "DALHAL_OneWireTempAutoRefresh.h"

#include <Arduino.h> // Needed for String class
#include <OneWire.h>
#include <DallasTemperature.h>

#include <ArduinoJson.h>

#include <DALHAL/Core/Device/DALHAL_Device.h>
#include <DALHAL/Core/Types/DALHAL_Registry.h>

#include <DALHAL/Config/DALHAL_ReactiveConfig.h>
#if USING_REACTIVE(ONE_WIRE_TEMP_BUS)
#include "DALHAL_OneWireTempBus_Reactive.h"
using OneWireTempBus_DeviceBase = DALHAL::OneWireTempBus_Reactive;
#else
using OneWireTempBus_DeviceBase = DALHAL::Device;
#endif

namespace DALHAL {

    class OneWireTempBus : public OneWireTempBus_DeviceBase {
    public: // public static fields and exposed external structures
        //static bool VerifyJSON(const JsonVariant &jsonObj);

    private:
        uint8_t pin;
        OneWire* oneWire = nullptr;
        DallasTemperature* dTemp = nullptr;
        bool haveDeviceWithRomID(OneWireAddress addr);

        int deviceCount = 0;
        Device **devices;

    public:    
        OneWireTempBus(DeviceCreateContext& context);
        ~OneWireTempBus() override;

        /** this function will search the devices to find the device with the uid */
        DeviceFindResult findDevice(UIDPath& path, Device*& outDevice) override;
        void requestTemperatures();
        void readAll();
        HALOperationResult read(const HALReadStringRequestValue& val) override;

        std::string getAllDevices(bool printTemp = false, bool onlyNewDevices = false);
        String ToString() override;
    };

    class OneWireTempBusAtRoot : public OneWireTempBus {
    public: // public static fields and exposed external structures
        static const Registry::DefineBase RegistryDefine;
        static Device* Create(DeviceCreateContext& context);

    private:
        OneWireTempAutoRefresh autoRefresh;

    public:
        OneWireTempBusAtRoot(DeviceCreateContext& context);
        ~OneWireTempBusAtRoot() override;

        void loop() override;
        
        String ToString() override;
    };
}