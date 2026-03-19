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

#include <ArduinoJson.h>
#include <DHTesp.h>

#include <DALHAL/Core/Types/DALHAL_Registry.h>
#include <DALHAL/Core/Device/DALHAL_Device.h>

 /* set to 2 sec to be safe, this also defines the minimum refreshrate possible */
#define DALHAL_TYPE_DHT_DEFAULT_REFRESHRATE_MS 2000

#include <DALHAL/Config/DALHAL_ReactiveConfig.h>
#if USING_REACTIVE(DHT)
#include "DALHAL_DHT_Reactive.h"
using DHTDeviceBase = DALHAL::DHT_Reactive;
#else
using DHTDeviceBase = DALHAL::Device;
#endif

namespace DALHAL {

    class DHT : public DHTDeviceBase {
    public: // public static fields and exposed external structures
        static const Registry::DefineBase RegistryDefine;
        static Device* Create(DeviceCreateContext& context);

    private:
        static bool isValidDHTModel(const char* model);
        static HALOperationResult readTemperature(Device* context, HALValue &val);
        static HALOperationResult readHumidity(Device* context, HALValue &val);

        DHTesp dht;
        uint8_t pin = 0;
        TempAndHumidity data;
        uint32_t refreshTimeMs = DALHAL_TYPE_DHT_DEFAULT_REFRESHRATE_MS;
        uint32_t lastUpdateMs = 0;
        
    public:
        DHT(DeviceCreateContext& context);

        String ToString() override;
        
        void loop() override; // will need loop for automatic polling as this device is slow

        ReadToHALValue_FuncType GetReadToHALValue_Function(ZeroCopyString& zcFuncName) override;
        
        HALOperationResult read(HALValue &val) override;
        HALOperationResult read(const HALReadValueByCmd &val) override;
        HALOperationResult read(const HALReadStringRequestValue &val) override;
    };

}