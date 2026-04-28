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

#include "DALHAL_DHT.h"
#include <strings.h>

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>
#include <DALHAL/Core/Manager/DALHAL_GPIO_Manager.h>

#include <DALHAL/Support/DALHAL_Logger.h>
#include <DALHAL/Core/JsonConfig/DALHAL_ArduinoJSON_ext.h>

#include "DALHAL_DHT_JSON_Schema.h"

#include <math.h>

namespace DALHAL {
    
    __attribute__((used, externally_visible))
    constexpr Registry::DefineBase DHT::RegistryDefine = {
        Create,
        &JsonSchema::DHT::Root,
        DALHAL_REACTIVE_EVENT_TABLE(DHT)
    };
    //volatile const void* keep_DHT = &DALHAL::DHT::RegistryDefine;

    DHT::DHT(DeviceCreateContext& context) : DHTDeviceBase(context.deviceType) {
        JsonSchema::DHT::Extractors::Apply(context, this);
        lastUpdateMs = millis()-refreshTimeMs; // direct update
    }

    Device* DHT::Create(DeviceCreateContext& context) {
        return new DHT(context);
    }

    String DHT::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += this->Type;
        ret += "\"";
        ret += DeviceConstStrings::pin;
        ret += std::to_string(pin).c_str();
        ret += ",\"humidity\":";
        ret += std::to_string(data.humidity).c_str();
        ret += ",\"temperature\":";
        ret += std::to_string(data.temperature).c_str();
        return ret;
    }

    void DHT::loop() {
        uint32_t now = millis();
        if ((now - lastUpdateMs) >= refreshTimeMs) {
            lastUpdateMs = millis();
            TempAndHumidity tempData = dht.getTempAndHumidity(); // this could take up to 250mS (of what i have read, but the timing spec only make it to max ~23mS)
            if (!isnan(tempData.humidity) && !isnan(tempData.temperature)) {
                data = tempData;
#if HAS_REACTIVE_VALUE_CHANGE(DHT)
                triggerValueChange();
#endif
            } else {
                //Serial.println("could not read DHT sensor");
            }
        }
    }

    HALOperationResult DHT::read(HALValue &val) {
        val = data.humidity;
        return HALOperationResult::Success;
    }

    Device::ReadToHALValue_FuncType DHT::GetReadToHALValue_Function(ZeroCopyString& zcFuncName) {
        if (zcFuncName == "temp") {
            return DHT::readTemperature;
        } else if (zcFuncName == "humidity") {
            return DHT::readHumidity;
        }
        else {
            GlobalLogger.Warn(F("DHT::read - cmd not found: "), zcFuncName.ToString().c_str()); // this can then be read by getting the last entry from logger
            return nullptr;
        }
    }

    HALOperationResult DHT::readTemperature(Device* context, HALValue& val) {
        DHT* dht = static_cast<DHT*>(context);
        val = dht->data.temperature;
        return HALOperationResult::Success;
    }
    HALOperationResult DHT::readHumidity(Device* context, HALValue& val) {
        DHT* dht = static_cast<DHT*>(context);
        val = dht->data.humidity;
        return HALOperationResult::Success;
    }

    HALOperationResult DHT::read(const HALReadValueByCmd &val) {
        if (val.cmd == "temp") {
            val.out_value = data.temperature;
            return HALOperationResult::Success;
        } else if (val.cmd == "humidity") {
            val.out_value = data.humidity;
            return HALOperationResult::Success;
        }
        else {
            std::string stdStrCmd = val.cmd.ToString();
            GlobalLogger.Warn(F("DHT::read - cmd not found: "), stdStrCmd.c_str()); // this can then be read by getting the last entry from logger
            return HALOperationResult::UnsupportedCommand;
        }
    }

    HALOperationResult DHT::read(const HALReadStringRequestValue &val) {
        if (val.cmd == "temp") {
            val.out_value = "{\"temp\":" + std::to_string(data.temperature) + "}";
            return HALOperationResult::Success;
        } else if (val.cmd == "humidity") {
            val.out_value = "{\"humidity\":" + std::to_string(data.humidity) + "}";
            return HALOperationResult::Success;
        }
        else {
            std::string stdStrCmd = val.cmd.ToString();
            GlobalLogger.Warn(F("DHT::read - cmd not found: "), stdStrCmd.c_str()); // this can then be read by getting the last entry from logger
            return HALOperationResult::UnsupportedCommand;
        }
    }
    
}