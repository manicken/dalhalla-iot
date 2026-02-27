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

#include "DALHAL_DHT.h"
#include <strings.h>
#include "../Support/DALHAL_Logger.h"
#include "../Core/Device/DALHAL_JSON_Config_Defines.h"
#include "../Support/DALHAL_ArduinoJSON_ext.h"
#include "../Core/Manager/DALHAL_GPIO_Manager.h"

namespace DALHAL {

    bool DHT::isValidDHTModel(const char* modelStr) {
        return 
            (strcasecmp(modelStr, DALHAL_TYPE_DHT_MODEL_DHT11) == 0)  ||
            (strcasecmp(modelStr, DALHAL_TYPE_DHT_MODEL_DHT22) == 0) ||
            (strcasecmp(modelStr, DALHAL_TYPE_DHT_MODEL_AM2302) == 0) ||
            (strcasecmp(modelStr, DALHAL_TYPE_DHT_MODEL_RHT03) == 0);
    }

    DHT::DHT(const JsonVariant &jsonObj, const char* type) : SimpleEventDevice(type) {
        //const char* uidStr = jsonObj[DALHAL_KEYNAME_UID].as<const char*>();
        //uid = encodeUID(uidStr);
        uid = encodeUID(GetAsConstChar(jsonObj,DALHAL_KEYNAME_UID));
        //pin = jsonObj[DALHAL_KEYNAME_PIN].as<uint8_t>();
        pin = GetAsUINT32(jsonObj, DALHAL_KEYNAME_PIN);
        GPIO_manager::ReservePin(pin); // this is in most cases taken care of in VerifyJSON but there are situations where it's needed
        refreshTimeMs = ParseRefreshTimeMs(jsonObj, DALHAL_TYPE_DHT_DEFAULT_REFRESHRATE_MS);
        if (refreshTimeMs < DALHAL_TYPE_DHT_DEFAULT_REFRESHRATE_MS) refreshTimeMs = DALHAL_TYPE_DHT_DEFAULT_REFRESHRATE_MS;
        //const char* modelStr = jsonObj[DALHAL_KEYNAME_DHT_MODEL].as<const char*>();
        const char* modelStr = GetAsConstChar(jsonObj, DALHAL_KEYNAME_DHT_MODEL);
        DHTesp::DHT_MODEL_t model = DHTesp::DHT_MODEL_t::AUTO_DETECT; // auto detect will not be used but we can use it as a default here;
        if ((strcasecmp(modelStr, DALHAL_TYPE_DHT_MODEL_DHT11) == 0))
            model = DHTesp::DHT_MODEL_t::DHT11;
        else if ((strcasecmp(modelStr, DALHAL_TYPE_DHT_MODEL_DHT22) == 0))
            model = DHTesp::DHT_MODEL_t::DHT22;
        else if ((strcasecmp(modelStr, DALHAL_TYPE_DHT_MODEL_AM2302) == 0))
            model = DHTesp::DHT_MODEL_t::AM2302;
        else if ((strcasecmp(modelStr, DALHAL_TYPE_DHT_MODEL_RHT03) == 0))
            model = DHTesp::DHT_MODEL_t::RHT03;
        dht.setup(pin,model);
        lastUpdateMs = millis()-refreshTimeMs; // direct update
     }

    bool DHT::VerifyJSON(const JsonVariant &jsonObj) {
        if (ValidateJsonStringField(jsonObj, DALHAL_KEYNAME_DHT_MODEL) == false){ SET_ERR_LOC(DALHAL_ERROR_SOURCE_DHT_VERIFY_JSON); return false; }
        //const char* modelStr = jsonObj[DALHAL_KEYNAME_DHT_MODEL].as<const char*>();
        const char* modelStr = GetAsConstChar(jsonObj, DALHAL_KEYNAME_DHT_MODEL); // this take 8bytes less than above

        if (!modelStr || isValidDHTModel(modelStr) == false) {
            GlobalLogger.Error(F("DHT model invalid/missing: "), modelStr);
            return false;
        }
        return GPIO_manager::ValidateJsonAndCheckIfPinAvailableAndReserve(jsonObj, (static_cast<uint8_t>(GPIO_manager::PinFunc::OUT) | static_cast<uint8_t>(GPIO_manager::PinFunc::IN)));
    }

    Device* DHT::Create(const JsonVariant &jsonObj, const char* type) {
        return new DHT(jsonObj, type);
    }

    String DHT::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += type;
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
                triggerEvent();
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