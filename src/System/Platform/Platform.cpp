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

#include "Platform.h"

#if defined(DALHAL_PLATFORM_ESP32)
// only ESP-IDF APIs, so this works on both the Arduino core and the plain ESP-IDF backend
#include <esp_system.h>
#include <esp_heap_caps.h>
#include <sdkconfig.h> // CONFIG_IDF_TARGET_*
#include <esp_flash.h>
#elif defined(DALHAL_PLATFORM_ESP8266)
#include <Arduino.h>
#include <stdlib.h>
#include <user_interface.h> // system_get_rst_info
#elif defined(DALHAL_PLATFORM_PC)
#include <cstdlib>
#endif

namespace Platform {

    const char* BackendName() {
#if defined(DALHAL_BACKEND_ARDUINO)
        return "arduino";
#elif defined(DALHAL_BACKEND_ESPIDF)
        return "esp-idf";
#else
        return "pc";
#endif
    }

#if defined(DALHAL_PLATFORM_ESP32)

    void Restart() {
        esp_restart();
    }

    void Abort(const char* reason) {
        esp_system_abort(reason);
    }

    const char* ChipName() {
#if defined(CONFIG_IDF_TARGET_ESP32)
        return "ESP32";
#elif defined(CONFIG_IDF_TARGET_ESP32S2)
        return "ESP32-S2";
#elif defined(CONFIG_IDF_TARGET_ESP32S3)
        return "ESP32-S3";
#elif defined(CONFIG_IDF_TARGET_ESP32C2)
        return "ESP32-C2";
#elif defined(CONFIG_IDF_TARGET_ESP32C3)
        return "ESP32-C3";
#elif defined(CONFIG_IDF_TARGET_ESP32C5)
        return "ESP32-C5";
#elif defined(CONFIG_IDF_TARGET_ESP32C6)
        return "ESP32-C6";
#elif defined(CONFIG_IDF_TARGET_ESP32H2)
        return "ESP32-H2";
#elif defined(CONFIG_IDF_TARGET_ESP32P4)
        return "ESP32-P4";
#else
        return "ESP32-UNKNOWN";
#endif
    }

    uint32_t FreeHeap() {
        return heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    }

    uint32_t LargestFreeBlock() {
        return heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    }

    bool ResetWasCrash(bool includeWatchdogs) {
        esp_reset_reason_t reason = esp_reset_reason();
        if (reason == ESP_RST_PANIC) return true; // also Abort() / esp_system_abort
        if (includeWatchdogs) {
            return reason == ESP_RST_INT_WDT || reason == ESP_RST_TASK_WDT || reason == ESP_RST_WDT;
        }
        return false;
    }

    const char* ResetReasonStr() {
        switch (esp_reset_reason()) {
            case ESP_RST_POWERON:   return "power on";
            case ESP_RST_EXT:       return "external pin";
            case ESP_RST_SW:        return "software";
            case ESP_RST_PANIC:     return "panic";
            case ESP_RST_INT_WDT:   return "interrupt watchdog";
            case ESP_RST_TASK_WDT:  return "task watchdog";
            case ESP_RST_WDT:       return "other watchdog";
            case ESP_RST_DEEPSLEEP: return "deep sleep";
            case ESP_RST_BROWNOUT:  return "brownout";
            case ESP_RST_SDIO:      return "sdio";
            default:                return "unknown";
        }
    }

    uint32_t FlashSize() {
        uint32_t size = 0;
        if (esp_flash_get_size(nullptr, &size) != ESP_OK) return 0;
        return size;
    }

#elif defined(DALHAL_PLATFORM_ESP8266)

    void Restart() {
        ESP.restart();
        while (true) {}
    }

    void Abort(const char* reason) {
        (void)reason;
        abort();
    }

    const char* ChipName() {
        return "ESP8266";
    }

    uint32_t FreeHeap() {
        return ESP.getFreeHeap();
    }

    uint32_t LargestFreeBlock() {
        return ESP.getMaxFreeBlockSize();
    }

    bool ResetWasCrash(bool includeWatchdogs) {
        const rst_info* info = system_get_rst_info();
        if (info->reason == REASON_EXCEPTION_RST) return true;
        if (includeWatchdogs) {
            return info->reason == REASON_WDT_RST || info->reason == REASON_SOFT_WDT_RST;
        }
        return false;
    }

    const char* ResetReasonStr() {
        switch (system_get_rst_info()->reason) {
            case REASON_DEFAULT_RST:      return "power on";
            case REASON_WDT_RST:          return "hardware watchdog";
            case REASON_EXCEPTION_RST:    return "exception";
            case REASON_SOFT_WDT_RST:     return "software watchdog";
            case REASON_SOFT_RESTART:     return "software";
            case REASON_DEEP_SLEEP_AWAKE: return "deep sleep";
            case REASON_EXT_SYS_RST:      return "external pin";
            default:                      return "unknown";
        }
    }

    uint32_t FlashSize() {
        return ESP.getFlashChipRealSize();
    }

#elif defined(DALHAL_PLATFORM_PC)

    void Restart() {
        std::exit(0);
    }

    void Abort(const char* reason) {
        (void)reason;
        std::abort();
    }

    const char* ChipName() {
        return "PC";
    }

    uint32_t FreeHeap() {
        return 0; // not meaningful on PC
    }

    uint32_t LargestFreeBlock() {
        return 0;
    }

    bool ResetWasCrash(bool includeWatchdogs) {
        (void)includeWatchdogs;
        return false;
    }

    const char* ResetReasonStr() {
        return "n/a";
    }

    uint32_t FlashSize() {
        return 0;
    }

#endif

}
