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

#include <ArduinoJson.h>
#include <DHTesp.h>

#include "HAL_JSON_DeviceTypesRegistry.h"
#include "../Core/HAL_JSON_Device.h"
#include "../Core/HAL_JSON_SimpleEventDevice.h"



#define HAL_JSON_KEYNAME_DHT_MODEL          "model"
// DHT models
#define HAL_JSON_TYPE_DHT_MODEL_DHT11       "DHT11"
#define HAL_JSON_TYPE_DHT_MODEL_DHT22       "DHT22"
#define HAL_JSON_TYPE_DHT_MODEL_AM2302      "AM2302"
#define HAL_JSON_TYPE_DHT_MODEL_RHT03       "RTH03"
 /* set to 2 sec to be safe, this also defines the minimum refreshrate possible */
#define HAL_JSON_TYPE_DHT_DEFAULT_REFRESHRATE_MS 2000

namespace HAL_JSON {

    class DHT : public SimpleEventDevice {
    private:
        DHTesp dht;
        uint8_t pin = 0;
        TempAndHumidity data;
        uint32_t refreshTimeMs = HAL_JSON_TYPE_DHT_DEFAULT_REFRESHRATE_MS;
        uint32_t lastUpdateMs = 0;
        static bool isValidDHTModel(const char* model);
    public:
        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(const JsonVariant &jsonObj, const char* type);
        static constexpr DeviceRegistryDefine RegistryDefine = {
            UseRootUID::Mandatory,
            Create,
            VerifyJSON
        };
        DHT(const JsonVariant &jsonObj, const char* type);

        String ToString() override;
        
        void loop() override; // will need loop for automatic polling as this device is slow

        static HALOperationResult readTemperature(Device* context, HALValue &val);
        static HALOperationResult readHumidity(Device* context, HALValue &val);
        ReadToHALValue_FuncType GetReadToHALValue_Function(ZeroCopyString& zcFuncName) override;
        

        HALOperationResult read(HALValue &val) override;
        HALOperationResult read(const HALReadValueByCmd &val) override;
        HALOperationResult read(const HALReadStringRequestValue &val) override;
    };

}