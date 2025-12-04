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

#include "../../../Support/Logger.h"
#include "../../../Support/ConvertHelper.h"
#include "../../HAL_JSON_Device.h"
#include "../../HAL_JSON_Device_GlobalDefines.h"
#include "../../HAL_JSON_ArduinoJSON_ext.h"
#include "HAL_JSON_OneWireTempDevice.h"
#include "HAL_JSON_OneWireTempAutoRefresh.h"
#include "../HAL_JSON_DeviceTypesRegistry.h"

namespace HAL_JSON {

    class OneWireTempBus : public Device {
    private:
        uint8_t pin;
        OneWire* oneWire = nullptr;
        DallasTemperature* dTemp = nullptr;
        bool haveDeviceWithRomID(OneWireAddress addr);
    public:
        int deviceCount = 0;
        OneWireTempDevice **devices;

        static bool VerifyJSON(const JsonVariant &jsonObj);
        OneWireTempBus(const JsonVariant &jsonObj, const char* type);
        ~OneWireTempBus();
        
        OneWireTempDevice* GetFirstDevice();
        /** this function will search the devices to find the device with the uid */
        DeviceFindResult findDevice(UIDPath& path, Device*& outDevice) override;
        void requestTemperatures();
        void readAll();
        HALOperationResult read(const HALReadStringRequestValue& val) override;

        std::string getAllDevices(bool printTemp = false, bool onlyNewDevices = false);
        String ToString() override;
    };

    class OneWireTempBusAtRoot : public OneWireTempBus {
    private:
        OneWireTempAutoRefresh autoRefresh;

    public:
        
        static Device* Create(const JsonVariant &jsonObj, const char* type);
        static constexpr DeviceRegistryDefine RegistryDefine = {
            UseRootUID::Optional,
            Create,
            &OneWireTempBus::VerifyJSON
        };
        OneWireTempBusAtRoot(const JsonVariant &jsonObj, const char* type);
        ~OneWireTempBusAtRoot();

        void loop() override;
        
        String ToString() override;
    };
}