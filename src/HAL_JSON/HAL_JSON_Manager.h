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

#define HAL_JSON_H_

#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#if defined(ESP32) || defined(ESP8266)
#include "../Support/LittleFS_ext.h"
#else
#include <LittleFS_ext.h>
#endif
#include <string>
#include "HAL_JSON_Value.h"
#include "HAL_JSON_UID_Path.h"
#include "HAL_JSON_Operations.h"
#include "HAL_JSON_Device.h"

#include "HAL_JSON_Device_GlobalDefines.h"
#include "Devices/HAL_JSON_DeviceTypesRegistry.h"

#include "../Support/Logger.h"
#include "HAL_JSON_ArduinoJSON_ext.h"

#define HAL_JSON_ROOT_URL                    "/hal"
#define HAL_JSON_FILES_PATH                  F(HAL_JSON_ROOT_URL)
#define HAL_JSON_CONFIG_JSON_FILE            F(HAL_JSON_ROOT_URL "/cfg.json")

namespace HAL_JSON {
    class Manager {
    private:
        static Device** devices;
        static int deviceCount;
        
        
        
        static int reloadVersion;

    public:
        static Device* CreateDeviceFromJSON(const JsonVariant& json);
        static bool VerifyDeviceJson(const JsonVariant& jsonObj);
        
        static bool reloadQueued;// = false;
        // getters
        static int DeviceCount();
        static int* ReloadVersionPtr();
        // init
        /** calls the begin function on all loaded hal devices */
        static void begin();
        static bool setupMgr();
        // JSON I/O
        static bool ParseJSON(const JsonArray& jsonArray);
        static bool ReadJSON(const char* path);
        static void CleanUp();

        // Device operations
        static DeviceFindResult findDevice(UIDPath& path, Device*& outDevice);

        // Maintenance
        static void loop();
        
        static std::string ToString();

        // Debug / Testing
        static void TEST();
    };
}