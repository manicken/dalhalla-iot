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

#include "DALHAL_ThingSpeak.h"

#include "../../Core/Manager/DALHAL_DeviceManager.h"
#include "../../Support/DALHAL_Logger.h"
#include "../../Core/Device/DALHAL_JSON_Config_Defines.h"
#include "../../Support/DALHAL_ArduinoJSON_ext.h"


#if __has_include("../../../secrets/thingspeak_test_server.h")
#include "../../../secrets/thingspeak_test_server.h"
#endif

#define DEBUG_UART Serial

namespace DALHAL {

    ThingSpeakField::ThingSpeakField() :
        index(0), 
        cdr(nullptr) {}

    ThingSpeakField::~ThingSpeakField() {
        if (deviceEvent != nullptr) {
            delete deviceEvent;
        }
    }

    void ThingSpeakField::Set(int fieldIndex, const char* uidPathAndFuncName_cStr, bool _sendAllInSync) {
        this->sendAllInSync = _sendAllInSync;
        this->index = fieldIndex;
        cdr = new CachedDeviceRead();
        ZeroCopyString zcStrUidPathAndFuncName(uidPathAndFuncName_cStr);
        if (cdr->Set(zcStrUidPathAndFuncName)) {
            DeviceManager::GetDeviceEvent(zcStrUidPathAndFuncName, &deviceEvent);
        }
        
    }


    const char* ThingSpeak::TS_ROOT_URL = "http://api.thingspeak.com/update?api_key=";
    
    ThingSpeak::ThingSpeak(const JsonVariant &jsonObj, const char* type) : Device(type) {
        const char* uidStr = GetAsConstChar(jsonObj,DALHAL_KEYNAME_UID);
        uid = encodeUID(uidStr);

        refreshTimeMs = ParseRefreshTimeMs(jsonObj, 0);
        if (refreshTimeMs != 0) {
            useOwnTaskLoop = true;
            //if (refreshTimeMs < 1000) refreshTimeMs = 1000;
        } else {
            useOwnTaskLoop = false;
        }
        
        if (jsonObj["testserver"].as<bool>()) {
#ifdef DALHAL_DEVICES_THINGSPEAK_TEST_SERVER
            ThingSpeak::TS_ROOT_URL = "http://" DALHAL_DEVICES_THINGSPEAK_TEST_SERVER ":8083/update?api_key=";
#else
            ThingSpeak::TS_ROOT_URL = "http://127.0.0.1:8083/update?api_key=";
#endif
        } else {
            ThingSpeak::TS_ROOT_URL = "http://api.thingspeak.com/update?api_key=";
        }


        const char* keyStr = GetAsConstChar(jsonObj, "key");
        
        strncpy(API_KEY, keyStr, sizeof(API_KEY) - 1);
        API_KEY[sizeof(API_KEY) - 1] = '\0'; // ensure null-termination

        JsonObject items = jsonObj[DALHAL_KEYNAME_ITEMS];
        fieldCount = items.size();
        fields = new ThingSpeakField[fieldCount];
        int index = 0;
        for (const JsonPair& kv : items) {
            int fieldIndex = atoi(kv.key().c_str());
            const char* uidPathAndFuncName_cStr = kv.value().as<const char*>();
            fields[index++].Set(fieldIndex, uidPathAndFuncName_cStr);
        }
        urlApi.reserve(strlen(TS_ROOT_URL) + strlen(API_KEY) + fieldCount*(strlen("&fieldx=")+32));

        if (useOwnTaskLoop) {
            uint32_t firstUpdateAfterSeconds = jsonObj["firstUpdateAfterSeconds"].as<uint32_t>();
            if (firstUpdateAfterSeconds == 0) {
                lastUpdateMs = millis() - refreshTimeMs; // force a first update directly
            } else {
                lastUpdateMs = millis() - firstUpdateAfterSeconds*1000; // force a first update after timeout
            }
        }
        
    }

    ThingSpeak::~ThingSpeak() {
        delete[] fields;
    }

    void ThingSpeak::loop() {
        if (useOwnTaskLoop == false) return;
        
        if (WiFi.status() != WL_CONNECTED) { return; } // need some timer to print this otherwise it will just flood the Serial port Serial.println("WiFi not connected, skipping ThingSpeak task"); }

        uint32_t now = millis();
        if ((now - lastUpdateMs) > refreshTimeMs) {
            lastUpdateMs = now;
            HALOperationResult res = this->exec();
        }
    }

    HALOperationResult ThingSpeak::exec() {
        //std::string urlApi;
        bool hasUpdates = false;
        urlApi.clear();
        urlApi += TS_ROOT_URL;
        urlApi += API_KEY;
        for (int i=0;i<fieldCount;i++) {
            ThingSpeakField& field = fields[i];
            
            if (field.DataReady() == false) continue; // skip values that have not currently been changed
            HALValue val;
            HALOperationResult hres = field.cdr->ReadSimple(val);
            if (hres != HALOperationResult::Success) continue;
            if (val.isNaN()) continue; // allows devices that have not been read currently to signal not set values
            
            urlApi += "&field";
            urlApi += (char)(field.index + 0x30); // safe as field.index never exceed 8
            urlApi += '=';
            val.appendToString(urlApi);
            hasUpdates = true;
            delay(0);
        }
        if (hasUpdates == false) {
            return HALOperationResult::ExecutionFailed;
        }

        http.begin(wifiClient, urlApi.c_str());
        http.setTimeout(2000); // set to 2 seconds so WDT would not trigger
                
        int httpCode = http.GET();
        if (httpCode <= 0) {
            DEBUG_UART.println(F("ERROR - ThingSpeak post\r\n"));
        }
        /*else {
            DEBUG_UART.println(F("[OK]\r\n"));
        }*/

        http.end();
        delay(0);
        return HALOperationResult::Success;
    }

    bool ThingSpeak::VerifyJSON(const JsonVariant &jsonObj) {
        if (!ValidateJsonStringField(jsonObj, DALHAL_KEYNAME_UID)){ SET_ERR_LOC(DALHAL_ERROR_SOURCE_THINGSPEAK_VERIFY_JSON); return false; }
        if (!ValidateJsonStringField(jsonObj, "key")){ SET_ERR_LOC(DALHAL_ERROR_SOURCE_THINGSPEAK_VERIFY_JSON); return false; }

        const char* keyStr = GetAsConstChar(jsonObj, "key");
        if (strlen(keyStr) != 16) {
            GlobalLogger.Error(F("key lenght != 16"));
            SET_ERR_LOC(DALHAL_ERROR_SOURCE_THINGSPEAK_VERIFY_JSON);
            return false;
        }
        
        if (jsonObj.containsKey(DALHAL_KEYNAME_ITEMS) == false) {
            GlobalLogger.Error(DALHAL_ERR_MISSING_KEY(DALHAL_KEYNAME_ITEMS));
            SET_ERR_LOC(DALHAL_ERROR_SOURCE_THINGSPEAK_VERIFY_JSON);
            return false;
        }
        
        if (jsonObj[DALHAL_KEYNAME_ITEMS].is<JsonObject>() == false) {
            GlobalLogger.Error(DALHAL_ERR_VALUE_TYPE(DALHAL_KEYNAME_ITEMS " not a object"));
            SET_ERR_LOC(DALHAL_ERROR_SOURCE_THINGSPEAK_VERIFY_JSON);
            return false;
        }
        JsonObject items = jsonObj[DALHAL_KEYNAME_ITEMS];
        if (items.size() == 0) {
            GlobalLogger.Error(F("items object is empty"));
            SET_ERR_LOC(DALHAL_ERROR_SOURCE_THINGSPEAK_VERIFY_JSON);
            return false;
        }
        for (JsonPair kv : items) {
            const char* indexStr = kv.key().c_str();
            const char* valueStr = kv.value().as<const char*>();

            // validate that index is numeric
            for (const char* p = indexStr; *p; p++) {
                if (!isdigit(*p)) {
                    GlobalLogger.Error(F("Invalid item index: "), indexStr);
                    SET_ERR_LOC(DALHAL_ERROR_SOURCE_THINGSPEAK_VERIFY_JSON);
                    return false;
                }
            }

            int fieldIndex = atoi(indexStr);
            if (fieldIndex < 1 || fieldIndex > DALHAL_THINGSPEAK_MAX_FIELDS) {
                GlobalLogger.Error(F("Invalid field index: "), indexStr);
                SET_ERR_LOC(DALHAL_ERROR_SOURCE_THINGSPEAK_VERIFY_JSON);
                return false;
            }

            // validate that value is non-empty
            if (valueStr == nullptr || *valueStr == '\0') {
                GlobalLogger.Error(F("Empty item value for index: "), indexStr);
                SET_ERR_LOC(DALHAL_ERROR_SOURCE_THINGSPEAK_VERIFY_JSON);
                return false;
            }

            // validate that the device exists, 
            // cannot be done here as devices are not loaded at this stage
            // and we do actually need the device instances to make the checks
            // to fix it the whole system need complete rewrite
            /*CachedDeviceRead cdr;
            ZeroCopyString zcStrUidPathAndFuncName(valueStr);
            if (cdr.Set(zcStrUidPathAndFuncName) == false) {
                GlobalLogger.Error(F("could not find device: "), indexStr);
                SET_ERR_LOC(DALHAL_ERROR_SOURCE_THINGSPEAK_VERIFY_JSON);
                return false;
            }*/
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