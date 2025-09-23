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

#include "HAL_JSON_ThingSpeak.h"

namespace HAL_JSON {

    const char ThingSpeak::TS_ROOT_URL[] = "http://api.thingspeak.com/update?api_key=";
    
    ThingSpeak::ThingSpeak(const JsonVariant &jsonObj, const char* type) : Device(UIDPathMaxLength::One,type) {
        const char* uidStr = GetAsConstChar(jsonObj,HAL_JSON_KEYNAME_UID);
        uid = encodeUID(uidStr);
        const char* keyStr = GetAsConstChar(jsonObj, "key");
        
        strncpy(API_KEY, keyStr, sizeof(API_KEY) - 1);
        API_KEY[sizeof(API_KEY) - 1] = '\0'; // ensure null-termination

        JsonObject items = jsonObj[HAL_JSON_KEYNAME_ITEMS];
        fieldCount = items.size();
        fields = new ThingSpeakField[fieldCount];
        int index = 0;
        for (JsonPair kv : items) {
            const char* indexStr = kv.key().c_str();
            const char* valueStr = kv.value().as<const char*>();
            ThingSpeakField& field = fields[index++];
            field.index = atoi(indexStr);

            ZeroCopyString zcFuncName(valueStr);
            ZeroCopyString zcPath = zcFuncName.SplitOffHead('#');
            field.cda = new CachedDeviceAccess(zcPath, zcFuncName);
        }
    }

    ThingSpeak::~ThingSpeak() {
        delete[] fields;
    }

    HALOperationResult ThingSpeak::exec() {
        std::string urlApi;
        urlApi += TS_ROOT_URL;
        urlApi += API_KEY;
        for (int i=0;i<fieldCount;i++) {
            ThingSpeakField& field = fields[i];
            HALValue val;
            HALOperationResult hres = field.cda->ReadSimple(val);
            if (hres != HALOperationResult::Success) continue;
            
            urlApi += "&field";
            urlApi += std::to_string(field.index);
            urlApi += '=';
            urlApi += val.toString();
        }
        http.begin(wifiClient, urlApi.c_str());
                
        int httpCode = http.GET();
        if (httpCode > 0) DEBUG_UART.println(F("[OK]\r\n"));
        else DEBUG_UART.println(F("[FAIL]\r\n"));

        http.end();
        return HALOperationResult::Success;
    }

    bool ThingSpeak::VerifyJSON(const JsonVariant &jsonObj) {
        if (!ValidateJsonStringField(jsonObj, HAL_JSON_KEYNAME_UID)){ SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_THINGSPEAK_VERIFY_JSON); return false; }
        if (!ValidateJsonStringField(jsonObj, "key")){ SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_THINGSPEAK_VERIFY_JSON); return false; }

        const char* keyStr = GetAsConstChar(jsonObj, "key");
        if (strlen(keyStr) != 16) {
            GlobalLogger.Error(F("key lenght != 16"));
            SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_THINGSPEAK_VERIFY_JSON);
            return false;
        }
        
        if (jsonObj.containsKey(HAL_JSON_KEYNAME_ITEMS) == false) {
            GlobalLogger.Error(HAL_JSON_ERR_MISSING_KEY(HAL_JSON_KEYNAME_ITEMS));
            SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_THINGSPEAK_VERIFY_JSON);
            return false;
        }
        
        if (jsonObj[HAL_JSON_KEYNAME_ITEMS].is<JsonObject>() == false) {
            GlobalLogger.Error(HAL_JSON_ERR_VALUE_TYPE(HAL_JSON_KEYNAME_ITEMS " not a object"));
            SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_THINGSPEAK_VERIFY_JSON);
            return false;
        }
        JsonObject items = jsonObj[HAL_JSON_KEYNAME_ITEMS];
        if (items.size() == 0) {
            GlobalLogger.Error(F("items object is empty"));
            SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_THINGSPEAK_VERIFY_JSON);
            return false;
        }
        for (JsonPair kv : items) {
            const char* indexStr = kv.key().c_str();
            const char* valueStr = kv.value().as<const char*>();

            // validate that index is numeric
            for (const char* p = indexStr; *p; p++) {
                if (!isdigit(*p)) {
                    GlobalLogger.Error(F("Invalid item index: "), indexStr);
                    SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_THINGSPEAK_VERIFY_JSON);
                    return false;
                }
            }

            int fieldIndex = atoi(indexStr);
            if (fieldIndex < 1 || fieldIndex > 8) {
                GlobalLogger.Error(F("Invalid field index: "), indexStr);
                SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_THINGSPEAK_VERIFY_JSON);
                return false;
            }

            // validate that value is non-empty
            if (valueStr == nullptr || *valueStr == '\0') {
                GlobalLogger.Error(F("Empty item value for index: "), indexStr);
                SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_THINGSPEAK_VERIFY_JSON);
                return false;
            }
        }
        
        return true;
    }

    Device* ThingSpeak::Create(const JsonVariant &jsonObj, const char* type) {
        return new ThingSpeak(jsonObj, type);
    }

    

    String ThingSpeak::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += type;
        ret += "\"";
        return ret;
    }

}