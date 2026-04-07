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

#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <stdlib.h>

#include <LittleFS.h>
#if defined(ESP32) || defined(ESP8266)
#include <Support/LittleFS_ext.h>
#else
#include <LittleFS_ext.h>
#endif

#if defined(ESP8266)
#define DEBUG_UART Serial1
#define WIFI_getChipId() ESP.getChipId()
#define WIFI_CHIPID_PREFIX "ESP_"
#elif defined(ESP32)
#define DEBUG_UART Serial
#define WIFI_getChipId() (uint32_t)(ESP.getEfuseMac()>>32)
#define WIFI_CHIPID_PREFIX "ESP32_"
#endif

#define MAIN_CONFIG_FILES_PATH                  "/"
#define MAIN_CONFIG_CONFIG_JSON_FILE            "/cfg.json"

namespace MainConfig {
    extern String mDNS_name;
    extern std::string lastJSONread_Error;
    void begin();
    bool ReadJson();
    void SetDefault_mDNS_name();
    void OnReadJsonFail();
}