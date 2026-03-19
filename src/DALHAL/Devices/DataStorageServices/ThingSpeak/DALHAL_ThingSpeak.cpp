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

#include "DALHAL_ThingSpeak.h"

#include <DALHAL/Core/Manager/DALHAL_DeviceManager.h>
#include <DALHAL/Support/DALHAL_Logger.h>
#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>
#include <DALHAL/Core/JsonConfig/DALHAL_ArduinoJSON_ext.h>

#include "DALHAL_ThingSpeak_JSON_Scheme.h"

#if __has_include(<thingspeak_test_server.h>)
#include <thingspeak_test_server.h>
#endif

#define DEBUG_UART Serial

namespace DALHAL {

    constexpr Registry::DefineBase ThingSpeak::RegistryDefine = {
        Create,
        &JsonSchema::ThingSpeak,
        DALHAL_REACTIVE_EVENT_TABLE(THINGSPEAK)
    };

    const char* ThingSpeak::TS_ROOT_URL = "http://api.thingspeak.com/update?api_key=";
    
    ThingSpeak::ThingSpeak(DeviceCreateContext& context) : ThingSpeak_DeviceBase(context.deviceType) {
        const JsonVariant& jsonObj = *(context.jsonObjItem);
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
#if HAS_REACTIVE_EXEC(THINGSPEAK)
        triggerExec();
#endif
        return HALOperationResult::Success;
    }

    Device* ThingSpeak::Create(DeviceCreateContext& context) {
        return new ThingSpeak(context);
    }

    String ThingSpeak::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += this->Type;
        ret += "\"";
        return ret;
    }

}