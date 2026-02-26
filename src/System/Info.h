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
#include <LittleFS.h>
#include "../Support/LittleFS_ext.h"

#include "../Support/Time_ext.h"
#include <ESPAsyncWebServer.h>

#if defined(ESP8266)
//#include <ESP8266WebServer.h>
#define WEBSERVER_TYPE AsyncWebServer

#define WIFI_getChipId() ESP.getChipId()
#define WIFI_CHIPID_PREFIX "ESP_"
#elif defined(ESP32)
//#include "Support/fs_WebServer.h"
#define WEBSERVER_TYPE AsyncWebServer


#define WIFI_getChipId() ESP.getEfuseMac()
#define WIFI_CHIPID_PREFIX "ESP32_"
#endif

#define INFO_URL                       "/info"
#define INFO_URL_ESP_FREE_HEAP         "/esp/free_heap"
#define INFO_URL_ESP_LAST_RESET_REASON "/esp/last_reset_reason"
#ifdef ESP32
#include <esp_heap_caps.h>
#endif
namespace Info
{

    
#ifdef ESP32
    float getHeapFragmentation();
#endif
    //WEBSERVER_TYPE *webserver = nullptr;

    extern time_t startTime;

    void printESP_info(void);
    String getESP_info();
    void srv_handle_info(AsyncWebServerRequest *req);
    
    bool resetReason_is_crash();
    const char* getResetReasonStr();

    std::string GetHeapInfo();

    void PrintHeapInfo();

    void setup(WEBSERVER_TYPE &srv);

    /*
// called from setup() function
void printESP_info(void);
    */

bool resetReason_is_crash(bool includeWatchdogs);


const char* getResetReasonStr();

uint64_t reverseBytes(uint64_t value);

void srv_handle_info(AsyncWebServerRequest* req);
}