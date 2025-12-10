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

#ifndef OTA_H
#define OTA_H

#include <Arduino.h>
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266httpUpdate.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <HTTPUpdate.h>
#endif
#include <ArduinoOTA.h>

namespace OTA{

#if defined(ESP8266)
    #define DEBUG_UART Serial1
#elif defined(ESP32)
    #define DEBUG_UART Serial
#endif

    
    void setup();
    void Download_Update(String url);
    void setup_PushedOTA(void);
    
    
}
#endif