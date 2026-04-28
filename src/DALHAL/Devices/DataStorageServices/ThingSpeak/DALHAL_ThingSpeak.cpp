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
#if defined (ESP8266)
#include <ESP8266WiFi.h>
#elif defined (ESP32)
#include <WiFi.h>
#endif

#if __has_include(<thingspeak_test_server.h>)
#include <thingspeak_test_server.h>
#endif

#include <DALHAL/Support/DALHAL_Logger.h>

#include "DALHAL_ThingSpeak_JSON_Schema.h"


namespace DALHAL {

    __attribute__((used, externally_visible))
    constexpr Registry::DefineBase ThingSpeak::RegistryDefine = {
        Create,
        &JsonSchema::ThingSpeak::Root,
        DALHAL_REACTIVE_EVENT_TABLE(THINGSPEAK)
    };
    //volatile const void* keep_ThingSpeak = &DALHAL::ThingSpeak::RegistryDefine;

    Device* ThingSpeak::Create(DeviceCreateContext& context) {
        return new ThingSpeak(context);
    }
    
    ThingSpeak::ThingSpeak(DeviceCreateContext& context) : ThingSpeak_DeviceBase(context.deviceType) {
        JsonSchema::ThingSpeak::Extractors::Apply(context, this);        
    }

    ThingSpeak::~ThingSpeak() {
        delete[] fields;
    }

    void ThingSpeak::loop() {
        if (useOwnTaskLoop == false) return;
#if defined(ESP8266) || defined(ESP32)
        if (WiFi.status() != WL_CONNECTED) { return; } // need some timer to print this otherwise it will just flood the Serial port Serial.println("WiFi not connected, skipping ThingSpeak task"); }
#endif
        uint32_t now = millis();
        if ((now - lastUpdateMs) > refreshTimeMs) {
            lastUpdateMs = now;
            /*HALOperationResult res = */this->exec();
        }
    }

    HALOperationResult ThingSpeak::exec() {
        //std::string urlApi;
        bool hasUpdates = false;
        urlApi.clear();
        urlApi += ts_root_url;
        urlApi += api_key;
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
            GlobalLogger.Error(F("ThingSpeak post"));
        }
        /*else {
            GlobalLogger.Info(F("ThingSpeak post [OK]"));
        }*/

        http.end();
        delay(0);
#if HAS_REACTIVE_EXEC(THINGSPEAK)
        triggerExec();
#endif
        return HALOperationResult::Success;
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