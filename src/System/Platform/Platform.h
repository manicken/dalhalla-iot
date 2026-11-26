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

/*
  Platform identification and the platform specific system API (layer E in docs/PLATFORM_CONTRACT.md).
  New code should test DALHAL_PLATFORM_* / DALHAL_HAS_* instead of compiler/core defines like ESP32.
*/

#include <stdint.h>
#include <stddef.h>

// backend: which framework provides the Arduino core subset (layer B in docs/PLATFORM_CONTRACT.md)
#if defined(ARDUINO)
#define DALHAL_BACKEND_ARDUINO
#elif defined(ESP_PLATFORM)
#define DALHAL_BACKEND_ESPIDF   // plain ESP-IDF, Arduino subset from ports/common + ports/esp-idf
#elif defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
#define DALHAL_BACKEND_PC       // Arduino subset from ports/common + ports/pc
#endif

// chip family
#if defined(ESP32)
#define DALHAL_PLATFORM_ESP32
#elif defined(ESP8266)
#define DALHAL_PLATFORM_ESP8266
#elif defined(ARDUINO_TEENSY40) || defined(ARDUINO_TEENSY41) || defined(ARDUINO_TEENSY_MICROMOD)
#define DALHAL_PLATFORM_TEENSY4
#elif defined(ARDUINO_ARCH_RP2040)
#define DALHAL_PLATFORM_RP2040
#elif defined(ARDUINO_ARCH_STM32)
#define DALHAL_PLATFORM_STM32
#elif defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
#define DALHAL_PLATFORM_PC
#else
#error "unknown DALHAL platform, add it to System/Platform/Platform.h"
#endif

// capabilities
#if defined(DALHAL_PLATFORM_ESP32) || defined(DALHAL_PLATFORM_ESP8266)
#define DALHAL_HAS_WIFI   // the Arduino WiFi API (arduino core, or ports/esp-idf/WiFi on plain ESP-IDF)
#endif

namespace Platform {

    /** restarts the MCU, on PC the simulation process exits */
    [[noreturn]] void Restart();
    /** aborts with a crash reset, used to force the failsafe/recovery mode on next boot */
    [[noreturn]] void Abort(const char* reason);

    /** chip name, e.g. "ESP32-C3", "ESP8266", "PC" */
    const char* ChipName();

    /** true when the last reset was a crash (panic/exception, optionally also watchdogs), used to enter failsafe mode */
    bool ResetWasCrash(bool includeWatchdogs);
    /** the reason of the last reset as text, e.g. "power on", "panic", "task watchdog" */
    const char* ResetReasonStr();

    /** size of the flash chip in bytes, 0 when unknown (PC) */
    uint32_t FlashSize();

    /** name of the backend that provides the Arduino subset: "arduino", "esp-idf" or "pc" */
    const char* BackendName();

    /** free internal heap in bytes */
    uint32_t FreeHeap();
    /** largest allocatable block of internal heap in bytes */
    uint32_t LargestFreeBlock();

}
