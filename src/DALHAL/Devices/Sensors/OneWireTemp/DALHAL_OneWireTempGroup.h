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

#include "DALHAL_OneWireTempBus.h"
#include "DALHAL_OneWireTempAutoRefresh.h"

#include <Arduino.h> // Needed for String class

#include <ArduinoJson.h>

#include <DALHAL/Core/Device/DALHAL_Device.h>
#include <DALHAL/Core/Types/DALHAL_Registry.h>

#include <DALHAL/Config/DALHAL_ReactiveConfig.h>
#if USING_REACTIVE(ONE_WIRE_TEMP_GROUP)
#include "DALHAL_OneWireTempGroup_Reactive.h"
using OneWireTempGroup_DeviceBase = DALHAL::OneWireTempGroup_Reactive;
#else
using OneWireTempGroup_DeviceBase = DALHAL::Device;
#endif

namespace DALHAL {

    class OneWireTempGroup : public OneWireTempGroup_DeviceBase {
    public:
        static const Registry::DefineRoot RegistryDefine;
        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(DeviceCreateContext& context);

    private:
        OneWireTempAutoRefresh autoRefresh;
        Device **busses;
        int busCount = 0;

        void requestTemperatures();
        void readAll();

    public:
        OneWireTempGroup(DeviceCreateContext& context);
        ~OneWireTempGroup();
        
        /** this function will search the busses and their devices to find the device with the uid */
        DeviceFindResult findDevice(UIDPath& path, Device*& outDevice) override;

        void loop() override;
        HALOperationResult read(const HALReadStringRequestValue &val) override;

        String ToString() override;

    };
}