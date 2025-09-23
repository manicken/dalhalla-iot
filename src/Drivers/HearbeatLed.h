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

#include <Arduino.h>

#if defined(ESP8266)
//#include <ESP8266WebServer.h>
#include <ESPAsyncWebServer.h>
#define DEBUG_UART Serial1
#define WEBSERVER_TYPE AsyncWebServer
#elif defined(ESP32)
//#include "Support/fs_WebServer.h"
#define DEBUG_UART Serial
//#define WEBSERVER_TYPE fs_WebServer
#include <ESPAsyncWebServer.h>
#define WEBSERVER_TYPE AsyncWebServer
#endif

#include "../Support/ConvertHelper.h"

namespace HeartbeatLed
{

#if defined (ESP32WROVER_E_IE)
    #define HEARTBEATLED_PIN 0
    #define HEARTBEATLED_ACTIVESTATE LOW
    #define HEARTBEATLED_INACTIVESTATE HIGH
    #define HEARTBEATLED_DEFAULT_ON_INTERVAL 100
    #define HEARTBEATLED_DEFAULT_OFF_INTERVAL 4000
#elif defined (ESP32DEV)
    #define HEARTBEATLED_PIN 2
    #define HEARTBEATLED_ACTIVESTATE HIGH
    #define HEARTBEATLED_INACTIVESTATE LOW
    #define HEARTBEATLED_DEFAULT_ON_INTERVAL 100
    #define HEARTBEATLED_DEFAULT_OFF_INTERVAL 4000
#elif defined (ESP8266)
    #define HEARTBEATLED_PIN 0
    #define HEARTBEATLED_ACTIVESTATE LOW
    #define HEARTBEATLED_INACTIVESTATE HIGH
    #define HEARTBEATLED_DEFAULT_ON_INTERVAL 100
    #define HEARTBEATLED_DEFAULT_OFF_INTERVAL 4000
#endif

    extern unsigned long HEARTBEATLED_ON_INTERVAL;
    extern unsigned long HEARTBEATLED_OFF_INTERVAL;

    void setup(WEBSERVER_TYPE &srv);
    void task(void);
    
}