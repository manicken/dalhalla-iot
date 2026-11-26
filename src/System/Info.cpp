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

#include "Info.h"

#include "System.h"
#include <DALHAL/Support/ConvertHelper.h>
#include <DALHAL/API/DALHAL_StringBuilderStreamer.h>
#include <System/Platform/Platform.h>
#include <BUILD_INFO.h>

#if defined(ESP8266)
#include <user_interface.h>
#endif

namespace Info
{

    
#ifdef ESP32
    float getHeapFragmentation() {
        // Get total and largest free block in 8-bit accessible memory
        size_t freeHeap = heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
        size_t largestBlock = heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);

        if (freeHeap == 0) return 0.0; // avoid division by zero

        // Fragmentation = 100 * (1 - (largest block / total free heap))
        return 100.0 * (1.0 - ((float)largestBlock / (float)freeHeap));
    }
#endif
    //WEBSERVER_TYPE *webserver = nullptr;

    time_t startTime = 0;

    void printESP_info(void);
    //void srv_handle_info(AsyncWebServerRequest *req);
    
    bool resetReason_is_crash();
    const char* getResetReasonStr();

    std::string GetHeapInfo() {
        std::string ret;
        
#if defined(ESP32)
        ret += "Free Heap:" + std::to_string(heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
        ret += "\nLargest Free Heap chunk:" + std::to_string(heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
#elif defined(ESP8266)
        ret += "Free Heap:" + std::to_string(ESP.getFreeHeap());
        //ret += "\nLargest Free Heap chunk:" + std::to_string(ESP.he());
#endif

#if defined(ESP8266)
        ret += "\nFragmentation:" + std::to_string(ESP.getHeapFragmentation());
#elif defined(ESP32)
        ret += "\nFragmentation:" + std::to_string(getHeapFragmentation());
#endif
        return ret;
    }

    void PrintHeapInfo() {
#if defined(ESP32)
        std::string ret = GetHeapInfo();
        printf("\n%s\n", ret.c_str());

#elif defined(ESP8266)
        Serial.printf_P(PSTR("Heap: %u, Max block: %u\n"),
            ESP.getFreeHeap(),
            ESP.getMaxFreeBlockSize());
        Serial1.printf_P(PSTR("Heap: %u, Max block: %u\n"),
            ESP.getFreeHeap(),
            ESP.getMaxFreeBlockSize());
#endif
        //heap_caps_print_heap_info(MALLOC_CAP_INTERNAL);
    }
/*
    void setup(WEBSERVER_TYPE &srv) {
        //webserver = &srv;

        srv.on(INFO_URL, srv_handle_info);
        srv.on(INFO_URL_ESP_LAST_RESET_REASON, [](AsyncWebServerRequest *req) {
            std::string resetInfo = "Last Reset at: " + Time_ext::GetTimeAsString(startTime);
            resetInfo += "\nReason: " + std::string(getResetReasonStr());
            
            req->send(200, F("text/plain"), resetInfo.c_str());
        });
        srv.on(INFO_URL_ESP_FREE_HEAP, [](AsyncWebServerRequest *req) {
            std::string ret = GetHeapInfo();
            req->send(200,F("text/plain"), ret.c_str());
        });
    }
*/

    bool resetReason_is_crash(bool includeWatchdogs)
    {
#if defined(ESP8266)
        rst_info *info = system_get_rst_info();
        uint32 reason = info->reason;
        if (reason == rst_reason::REASON_EXCEPTION_RST)
            return true;
        if (includeWatchdogs) {
            switch (reason) {
                case rst_reason::REASON_WDT_RST: return true;
                case rst_reason::REASON_SOFT_WDT_RST: return true;
                default: return false;
            }
        }
#elif defined(ESP32)
        esp_reset_reason_t reason = esp_reset_reason();
        if (reason == ESP_RST_PANIC) return true;
        if (includeWatchdogs) {
            switch (reason) {
                case ESP_RST_INT_WDT: return true;
                case ESP_RST_TASK_WDT: return true;
                case ESP_RST_WDT: return true;
                default: return false;
            }
        }
#endif
        return false;
    }

    const char* getResetReasonStr()
    {
#if defined(ESP8266)
        rst_info *info = system_get_rst_info();
        uint32 reason = info->reason;
        if (reason == rst_reason::REASON_DEFAULT_RST)
            return PSTR("normal startup by power on");
        else if (reason == rst_reason::REASON_WDT_RST)
            return PSTR("hardware watch dog reset");
        else if (reason == rst_reason::REASON_EXCEPTION_RST)
            return PSTR("exception reset");
        else if (reason == rst_reason::REASON_SOFT_WDT_RST)
            return PSTR("software watch dog reset");
        else if (reason == rst_reason::REASON_SOFT_RESTART)
            return PSTR("software restart/system_restart");
        else if (reason == rst_reason::REASON_DEEP_SLEEP_AWAKE)
            return PSTR("wake up from deep-sleep");
        else if (reason == rst_reason::REASON_EXT_SYS_RST)
            return PSTR("external system reset");
        else
            return PSTR("undefined reset cause");
#elif defined(ESP32)
        esp_reset_reason_t reset_reason = esp_reset_reason();
        switch (reset_reason) {
            case ESP_RST_POWERON: return PSTR("Power-on reset");
            case ESP_RST_EXT: return PSTR("External reset");
            case ESP_RST_SW: return PSTR("Software reset");
            case ESP_RST_PANIC: return PSTR("Software reset due to panic");
            case ESP_RST_INT_WDT: return PSTR("Interrupt watchdog reset");
            case ESP_RST_TASK_WDT: return PSTR("Task watchdog reset");
            case ESP_RST_WDT: return PSTR("Other watchdog reset");
            case ESP_RST_DEEPSLEEP: return PSTR("Reset after deep sleep");
            case ESP_RST_BROWNOUT: return PSTR("Brownout reset");
            case ESP_RST_SDIO: return PSTR("SDIO reset");
            default: return PSTR("Unknown reset reason");
        }
#endif
        return PSTR("undefined reset cause"); 
    }

    uint64_t reverseBytes(uint64_t value) {
        return ((value & 0x00000000000000FF) << 56) |
            ((value & 0x000000000000FF00) << 40) |
            ((value & 0x0000000000FF0000) << 24) |
            ((value & 0x00000000FF000000) << 8)  |
            ((value & 0x000000FF00000000) >> 8)  |
            ((value & 0x0000FF0000000000) >> 24) |
            ((value & 0x00FF000000000000) >> 40) |
            ((value & 0xFF00000000000000) >> 56);
    }

    void getInfo(DALHAL::StringBuilderStreamer& sbs) {
        // the same on every platform, the platform specific parts come from Platform::
        sbs.write_json_object_begin();
        sbs.write_jsonString(F("mcu"), Platform::ChipName());
        sbs.write_json_value_separator(); sbs.write_jsonString(F("framework"), Platform::BackendName());
        sbs.write_json_value_separator(); sbs.write_jsonString(F("build_version"), BUILD_VER_STR);
        sbs.write_json_value_separator(); sbs.write_jsonNumber(F("uptime_ms"), (uint32_t)millis());
        sbs.write_json_value_separator(); sbs.write_jsonString(F("reset_reason"), Platform::ResetReasonStr());
        sbs.write_json_value_separator(); sbs.write_jsonNumber(F("heap"), Platform::FreeHeap());
        sbs.write_json_value_separator(); sbs.write_jsonNumber(F("max_block"), Platform::LargestFreeBlock());
        sbs.write_json_value_separator(); sbs.write_jsonNumber(F("flash_size"), Platform::FlashSize());
#if defined(DALHAL_PLATFORM_ESP32) || defined(DALHAL_PLATFORM_ESP8266)
        uint32_t fsTotal = 0, fsUsed = 0;
#if defined(DALHAL_PLATFORM_ESP32)
        fsTotal = (uint32_t)LittleFS.totalBytes();
        fsUsed = (uint32_t)LittleFS.usedBytes();
#else
        FSInfo fsi;
        if (LittleFS.info(fsi)) { fsTotal = (uint32_t)fsi.totalBytes; fsUsed = (uint32_t)fsi.usedBytes; }
#endif
        sbs.write_json_value_separator(); sbs.write_jsonMemberStart(F("littlefs"));
        sbs.write_json_object_begin();
        sbs.write_jsonBool(F("mounted"), fsTotal > 0);
        sbs.write_json_value_separator(); sbs.write_jsonNumber(F("total_bytes"), fsTotal);
        sbs.write_json_value_separator(); sbs.write_jsonNumber(F("used_bytes"), fsUsed);
        sbs.write_json_object_end();
#endif
        sbs.write_json_object_end();
    }

}