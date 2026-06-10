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

#include <DALHAL/API/DALHAL_StringBuilderStreamer.h>

#include <DALHAL/Core/Types/DALHAL_Device.h>
#include <DALHAL/Core/Types/DALHAL_Registry.h>

#include <DALHAL/Core/Types/DALHAL_DeviceFunctionTable.h>

#include <DALHAL/Core/Reactive/DALHAL_ReactiveConfig.h>
#if USING_REACTIVE(ONE_WIRE_TEMP_BUS)
#include "DALHAL_OneWireTempBus_Reactive.h"
using OneWireTempBus_DeviceBase = DALHAL::OneWireTempBus_Reactive;
#else
using OneWireTempBus_DeviceBase = DALHAL::Device;
#endif

namespace DALHAL {

    namespace JsonSchema { namespace OneWireTempBus { struct Extractors; } } // forward declaration

    class OneWireTempBus : public OneWireTempBus_DeviceBase {
        friend struct JsonSchema::OneWireTempBus::Extractors; // allow access to private memebers of this class from the schema extractor

    public: // public static fields and exposed external structures
        static const Registry::DefineBase RegistryDefine;
        static const DeviceFunctionTable FunctionTable;

    private:
        static Device* Create(DeviceCreateContext& context);
        static const FunctionEntry<FunctionTypes::ReadString> readStringFunctions[];

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

        const Registry::DefineBase* GetRegistryDefine() override;

        /** this function will search the devices to find the device with the uid */
        DeviceFindResult findDevice(UIDPath& path, Device*& outDevice) override;
        void requestTemperatures();
        void readAll();
        HALOperationResult read(const HALReadStringRequestValue& val) override;

        void getAllDevices(bool printTemp, bool onlyNewDevices, StringBuilderStreamer& sbs);
        
        
        void PrintTo(StringBuilderStreamer& sbs) override;

        static HALOperationResult readString_getAllNewDevices_Function(Device* device, ZeroCopyString zcStrParameters, StringBuilderStreamer& sbs);
        static HALOperationResult readString_getAllNewDevicesWithTemp_Function(Device* device, ZeroCopyString zcStrParameters, StringBuilderStreamer& sbs);
        static HALOperationResult readString_getAllDevices_Function(Device* device, ZeroCopyString zcStrParameters, StringBuilderStreamer& sbs);
        static HALOperationResult readString_getAllTemperatures_Function(Device* device, ZeroCopyString zcStrParameters, StringBuilderStreamer& sbs);
    
    };

    namespace JsonSchema { namespace OneWireTempBusAtRoot { struct Extractors; } } // forward declaration

    class OneWireTempBusAtRoot : public OneWireTempBus {
        friend struct JsonSchema::OneWireTempBusAtRoot::Extractors; // allow access to private memebers of this class from the schema extractor

    public: // public static fields and exposed external structures
        static const Registry::DefineBase RegistryDefine;
    
    private:
        static Device* Create(DeviceCreateContext& context);

    private:
        OneWireTempAutoRefresh autoRefresh;

    public:
        OneWireTempBusAtRoot(DeviceCreateContext& context);
        ~OneWireTempBusAtRoot() override;

        const Registry::DefineBase* GetRegistryDefine() override;

        void loop() override;
        
        
        void PrintTo(StringBuilderStreamer& sbs) override;
        
    };
}