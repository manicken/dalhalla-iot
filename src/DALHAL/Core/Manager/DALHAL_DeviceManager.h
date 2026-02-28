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

#define DALHAL_H_

#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#if defined(ESP32) || defined(ESP8266)
#include "../../../Support/LittleFS_ext.h"
#else
#include <LittleFS_ext.h>
#endif
#include <string>
#include "../Types/DALHAL_Value.h"
#include "../Types/DALHAL_UID_Path.h"
#include "../Types/DALHAL_Operations.h"
#include "../Device/DALHAL_Device.h"



#define DALHAL_ROOT_URL                    "/hal"
#define DALHAL_FILES_PATH                  F(DALHAL_ROOT_URL)
#define DALHAL_CONFIG_JSON_FILE            F(DALHAL_ROOT_URL "/cfg.json")

namespace DALHAL {
    class DeviceManager {
    private:
        static Device** devices;
        static int deviceCount;

    public:
        static Device* CreateDeviceFromJSON(const JsonVariant& json);
        static bool VerifyDeviceJson(const JsonVariant& jsonObj);

        // getters
        static int DeviceCount();
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
        /** get the device event struct if the source device supports events, otherwise it returns nullptr, 
         * a special note the consumer MUST delete the DeviceEvent when done with it i.e using delete eventDevice
         */
        static HALOperationResult GetDeviceEvent(ZeroCopyString zcStrUidPathAndFuncName, ReactiveEvent** reactiveEventOut);
        static HALOperationResult ValidateDeviceEvent(ZeroCopyString zcStrUidPathAndFuncName);

        // Maintenance
        static void loop();
        
        static std::string ToString();

        // Debug / Testing
        static void TEST();
    };
}