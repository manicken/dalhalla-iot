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

#include <Arduino.h>
#include <ArduinoJson.h>
#include <stdlib.h>
#define FAN_H_

namespace FAN {

    uint8_t pin = 19;
    uint32_t freq = 25000;
    uint32_t value = 1;
    uint8_t resolution = 8;
    uint8_t inv_out = 1;

    void init();
    void DecodeFromJSON(std::string jsonStr);
    void DecodeFromJSON(JsonDocument &json);
    uint32_t getInvValue(uint32_t val);
    void SetFanSpeed(uint32_t val);

    void init()
    {
#ifdef ESP8266
        ESP.rtcUserMemoryRead(0, &value, 4);
#endif
        
#if defined(ESP8266)
        analogWriteResolution(resolution);
        analogWriteFreq(freq);
#elif defined(ESP32)
        
        //analogWriteFrequency(freq);
#endif
        if (inv_out)
            value = getInvValue(value);
        analogWrite(pin, value);
        
    }

    uint32_t getInvValue(uint32_t val)
    {
        if (resolution == 10)
            return 1023-val;
        else if (resolution == 8)
            return 255-val;

        return 255;
    }
    
    void SetFanSpeed(uint32_t val)
    {
        //Serial1.printf("from json:%d", val);
#ifdef ESP8266
        ESP.rtcUserMemoryWrite(0, &val, 4);
#endif
        
        if (inv_out)
            value = getInvValue(val);
        else
            value = val;
       // Serial1.printf("what to write:%d", value);
        analogWrite(pin, value);
    }

    void DecodeFromJSON(std::string jsonStr)
    {
        StaticJsonDocument<128> json;
        deserializeJson(json, jsonStr.c_str());
        DecodeFromJSON(json);
    }

    void DecodeFromJSON(JsonDocument &json)
    {
        char buff[160]; serializeJson(json, buff); DEBUG_UART.println(buff);
        if (json.containsKey("pin")) {
            pin = json["pin"];
        }
        if (json.containsKey("freq")) {
            freq = json["freq"];
#if defined(ESP8266)
            analogWriteFreq(freq);
#elif defined(ESP32)
           // analogWriteFrequency(freq);
#endif
        }
        if (json.containsKey("bits")) {
            resolution = json["bits"];
           // analogWriteResolution(resolution);
        }
        if (json.containsKey("val")) {
            SetFanSpeed(json["val"]);
        }
        if (json.containsKey("inv_out")) {
            inv_out = json["inv_out"];
        }
    }
}