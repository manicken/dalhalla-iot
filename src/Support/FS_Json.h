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

/**
  The filesystems of the device and their content as JSON, the same on every backend
  (Arduino ESP32/ESP8266, ESP-IDF). Used by the /fs/... HTTP routes (FS_Http), so the frontend
  gets the same answers from every firmware.

  A filesystem is selected by name (the ?fs= parameter): "littlefs" (internal flash, the default)
  or "sd" (SD card, ESP32 with SDMMC on the Arduino core, only when a card is mounted).
*/

#pragma once

#include <string>
#include <System/Platform/Platform.h>

#if defined(DALHAL_PLATFORM_ESP32) || defined(DALHAL_PLATFORM_ESP8266)

#if defined(DALHAL_BACKEND_ARDUINO)
#include <FS.h>
#if defined(DALHAL_PLATFORM_ESP32)
#include <soc/soc_caps.h>
#if SOC_SDMMC_HOST_SUPPORTED
#define FS_JSON_HAS_SD_MMC
#endif
#endif
#else
#include <LittleFS.h> // ports/esp-idf/LittleFS
#endif

namespace FS_Json {

#if defined(DALHAL_BACKEND_ARDUINO)
    using FsType = fs::FS;
#else
    using FsType = LittleFSFS;
#endif

    /** the filesystem for a name, "littlefs" when name is null or empty;
        nullptr when the name is unknown or that filesystem is not mounted */
    FsType* Get(const char* name);

    /** the mounted filesystems: [{"fs":"littlefs","total":983040,"used":389120},{"fs":"sd",...}] */
    void Mounts(std::string& out);

    /** true when the path is usable for the /fs/... routes: absolute, no "..", not too long */
    bool IsValidPath(const char* path);

    /** [{"name":"cfg.json","dir":false,"size":2482},{"name":"api","dir":true,"size":0}]
        false when the path is not a folder */
    bool List(FsType& fs, const char* path, std::string& out);
}

#endif
