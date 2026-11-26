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

// DALHAL on plain ESP-IDF, step 3b: config + script engine on LittleFS, no network yet

#include <Arduino.h>
#include <LittleFS.h>

#include <DALHAL/Core/Manager/DALHAL_DeviceManager.h>
#include <DALHAL/ScriptEngine/DALHAL_SCRIPT_ENGINE.h>
#include <DALHAL/ScriptEngine/Runtime/DALHAL_SCRIPT_ENGINE_ScriptBlocks.h>

extern "C" void app_main(void)
{
    printf("\nDALHAL on ESP-IDF\n");

    // same partition and mount point as the Arduino builds, so the same LittleFS image works
    if (!LittleFS.begin(false)) {
        printf("LittleFS mount failed, upload the filesystem image (data/ folder)\n");
    }

    if (DALHAL::DeviceManager::init()) {
        DALHAL::ScriptEngine::ValidateAndLoadAllActiveScripts();
    } else {
        printf("DeviceManager::init failed, see the log above\n");
    }

    while (true) {
        DALHAL::DeviceManager::loop();
        if (DALHAL::ScriptEngine::ScriptBlocks::running) {
            DALHAL::ScriptEngine::ScriptBlocks::Exec();
        }
        vTaskDelay(1); // 1 ms with CONFIG_FREERTOS_HZ=1000, lets the idle task feed the watchdog
    }
}
