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

#include "HAL_JSON_ArduinoJSON_ext.h"

namespace HAL_JSON {
    bool containsKeyIgnoreCase(JsonObject obj, const char* keyToFind) {
        for (JsonPair kv : obj) {
            if (strcasecmp(kv.key().c_str(), keyToFind) == 0) {
                return true;
            }
        }
        return false;
    }

    JsonVariant getValueIgnoreCase(JsonObject obj, const char* keyToFind) {
        for (JsonPair kv : obj) {
            if (strcasecmp(kv.key().c_str(), keyToFind) == 0) {
                return kv.value();
            }
        }
        return JsonVariant(); // null
    }

    bool ValidateJsonStringField(const JsonVariant &jsonObj, const char* keyName) {
        if (!jsonObj.containsKey(keyName)) {
            GlobalLogger.Error(HAL_JSON_ERR_MISSING_STRING_VALUE_KEY, keyName);
            return false;
        }
        if (!IsConstChar(jsonObj,keyName)) {
            GlobalLogger.Error(HAL_JSON_ERR_VALUE_TYPE_NOT_STRING, keyName);
            return false;
        }
        const char* val = jsonObj[keyName].as<const char*>();
        if (val == nullptr || strlen(val) == 0) {
            GlobalLogger.Error(HAL_JSON_ERR_STRING_EMPTY, keyName);
            return false;
        }
        return true;
    }

    bool ValidateJsonStringField_noContains(const JsonVariant &jsonObj, const char* keyName) {
        
        if (!IsConstChar(jsonObj,keyName)) {
            GlobalLogger.Error(HAL_JSON_ERR_VALUE_TYPE_NOT_STRING, keyName);
            return false;
        }
        const char* val = jsonObj[keyName].as<const char*>();
        if (val == nullptr || strlen(val) == 0) {
            GlobalLogger.Error(HAL_JSON_ERR_STRING_EMPTY, keyName);
            return false;
        }
        return true;
    }

    double ParseRefreshTime(const JsonVariant &value) {
        if (value.is<float>() || value.is<double>()) {
            return value.as<double>();
        } else if (IsConstChar(value)) {
            return atof(value.as<const char*>());
        } else if (value.is<uint32_t>()) {
            return static_cast<double>(value.as<uint32_t>());
        }
        return -1.0; // Invalid
    }

    uint32_t ParseRefreshTimeMs(const JsonVariant &jsonObj, const uint32_t defaultRefreshTimeMs) {
        double rawSec = 1.0;
        if (jsonObj.containsKey(HAL_JSON_KEYNAME_REFRESHTIME_SEC)) {
            double val = ParseRefreshTime(jsonObj[HAL_JSON_KEYNAME_REFRESHTIME_SEC]);
            if (val >= 1.0) rawSec = val;
        }
        else if (jsonObj.containsKey(HAL_JSON_KEYNAME_REFRESHTIME_MIN)) {
            double val = ParseRefreshTime(jsonObj[HAL_JSON_KEYNAME_REFRESHTIME_MIN]);
            val *= 60.0;
            if (val >= 1.0) rawSec = val;
        }
        else {
            std::string msg = std::to_string(defaultRefreshTimeMs);
            GlobalLogger.Warn(F("refreshrate is not set default is:"),msg.c_str());
            return defaultRefreshTimeMs;
        }
        return static_cast<uint32_t>(round(rawSec * 1000));
    }
    bool ValidateUINT8(const JsonVariant& jsonObj, const char* keyName) {
        if (!jsonObj.containsKey(keyName)) {
            GlobalLogger.Error(HAL_JSON_ERR_MISSING_STRING_VALUE_KEY, keyName);
            return false;
        }
        if (!jsonObj[keyName].is<uint8_t>()){
            GlobalLogger.Error(F("Value type: "), keyName);
            return false;
        }
        return true;
    }
    bool ValidateUINT32(const JsonVariant& jsonObj, const char* keyName) {
        if (!jsonObj.containsKey(keyName)) {
            GlobalLogger.Error(HAL_JSON_ERR_MISSING_STRING_VALUE_KEY, keyName);
            return false;
        }
        if (!jsonObj[keyName].is<uint32_t>()){
            GlobalLogger.Error(F("Value type: "), keyName);
            return false;
        }
        return true;
    }
    bool IsUINT32(const JsonVariant& jsonObj, const char* keyName) {
        return jsonObj[keyName].is<uint32_t>();
    }

    uint32_t GetAsUINT32(const JsonVariant& jsonObj, const char* keyName, uint32_t defaultValue) {
        return jsonObj[keyName] | defaultValue;
    }
    uint32_t GetAsUINT32(const JsonVariant& jsonObj, const char* keyName) {
        return jsonObj[keyName];
    }
    uint16_t GetAsUINT16(const JsonVariant& jsonObj, const char* keyName) {
        return jsonObj[keyName];
    }
    uint8_t GetAsUINT8(const JsonVariant& jsonObj, const char* keyName) {
        return jsonObj[keyName];
    }
    const char* GetAsConstChar(const JsonVariant& jsonObj, const char* keyName) {
        return jsonObj[keyName].as<const char*>();
    }
    

    
}