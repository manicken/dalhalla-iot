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

#include <ESPAsyncWebServer.h>

#define MAIN_URLS_FORMAT_LITTLE_FS      "/formatLittleFs"
#define MAIN_URLS_MKDIR                 "/mkdir"

#define AUTOFORMAT_ON_FAIL true

#if defined(ESP8266)
#define LITTLEFS_BEGIN_FUNC_CALL LittleFS.begin()
#elif defined(ESP32)
#define LITTLEFS_BEGIN_FUNC_CALL LittleFS.begin(AUTOFORMAT_ON_FAIL, "/LittleFS", 10, "spiffs")
#endif

namespace System {

    void Setup();

    #if defined(ESP32)
    bool InitSD_MMC();
    void Start_MDNS();
    #endif

    void failsafeLoop();

    void initWebServerHandlers(AsyncWebServer& webserver);

}