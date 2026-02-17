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

#include "Info.h"

#include "../System/System.h"

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
    void srv_handle_info(AsyncWebServerRequest *req);
    
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
        std::string ret = GetHeapInfo();
        printf("\n%s\n", ret.c_str());
        //heap_caps_print_heap_info(MALLOC_CAP_INTERNAL);
    }

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

    /*
// called from setup() function
void printESP_info(void) { 
    uint32_t realSize = ESP.getFlashChipRealSize();
    uint32_t ideSize = ESP.getFlashChipSize();
    FlashMode_t ideMode = ESP.getFlashChipMode();

    DEBUG_UART.print(F("Flash real id:   ")); DEBUG_UART.printf("%08X\r\n", ESP.getFlashChipId());
    DEBUG_UART.print(F("Flash real size: ")); DEBUG_UART.printf("%u 0\r\n\r\n", realSize);

    DEBUG_UART.print(F("Flash ide  size: ")); DEBUG_UART.printf("%u\r\n", ideSize);
    DEBUG_UART.print(F("Flash ide speed: ")); DEBUG_UART.printf("%u\r\n", ESP.getFlashChipSpeed());
    DEBUG_UART.print(F("Flash ide mode:  ")); DEBUG_UART.printf("%s\r\n", (ideMode == FM_QIO ? "QIO" : ideMode == FM_QOUT ? "QOUT" : ideMode == FM_DIO ? "DIO" : ideMode == FM_DOUT ? "DOUT" : "UNKNOWN"));

    if(ideSize != realSize)
    {
        DEBUG_UART.println(F("Flash Chip configuration wrong!\r\n"));
    }
    else
    {
        DEBUG_UART.println(F("Flash Chip configuration ok.\r\n"));
    }
    DEBUG_UART.printf(" ESP8266 Chip id = %08X\n", ESP.getChipId());
    DEBUG_UART.println();
    DEBUG_UART.println();
}*/

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
        return "normal startup by power on";
    else if (reason == rst_reason::REASON_WDT_RST)
        return "hardware watch dog reset";
    else if (reason == rst_reason::REASON_EXCEPTION_RST)
        return "exception reset";
    else if (reason == rst_reason::REASON_SOFT_WDT_RST)
        return "software watch dog reset";
    else if (reason == rst_reason::REASON_SOFT_RESTART)
        return "software restart/system_restart";
    else if (reason == rst_reason::REASON_DEEP_SLEEP_AWAKE)
        return "wake up from deep-sleep";
    else if (reason == rst_reason::REASON_EXT_SYS_RST)
        return "external system reset";
    else
        return "undefined reset cause";
#elif defined(ESP32)
    esp_reset_reason_t reset_reason = esp_reset_reason();
    switch (reset_reason) {
        case ESP_RST_POWERON: return "Power-on reset";
        case ESP_RST_EXT: return "External reset";
        case ESP_RST_SW: return "Software reset";
        case ESP_RST_PANIC: return "Software reset due to panic";
        case ESP_RST_INT_WDT: return "Interrupt watchdog reset";
        case ESP_RST_TASK_WDT: return "Task watchdog reset";
        case ESP_RST_WDT: return "Other watchdog reset";
        case ESP_RST_DEEPSLEEP: return "Reset after deep sleep";
        case ESP_RST_BROWNOUT: return "Brownout reset";
        case ESP_RST_SDIO: return "SDIO reset";
        default: return "Unknown reset reason";
    }
#endif
        
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

void srv_handle_info(AsyncWebServerRequest* req)
{
    String srv_return_msg = getESP_info();
    req->send(200, "text/html", srv_return_msg);
    //server.sendContent(srv_return_msg);

    //server.sendContent("");
}
String getESP_info() {
    uint32_t ideSize = ESP.getFlashChipSize();
#if defined(ESP8266)
    uint32_t realSize = ESP.getFlashChipRealSize();
#else
    uint32_t realSize = ideSize;
#endif
    FlashMode_t ideMode = ESP.getFlashChipMode();
    String infoMsg = "";

    infoMsg.concat(F("<!DOCTYPE html PUBLIC\"ISO/IEC 15445:2000//DTD HTML//EN\"><html><head><title></title></head><body>"));
#if defined(ESP8266)
    srv_return_msg.concat(F("<br>Flash real id:   ")); srv_return_msg.concat(ESP.getFlashChipId());
#elif defined(ESP32)
    infoMsg.concat(F("<br>Flash real id:  (ESP32 don't have this function)"));
#endif
    uint64_t macAddrBigEndian = Convert::reverseMACaddress(WIFI_getChipId());
    String hostString = String(macAddrBigEndian & 0xFFFFFF,HEX);
    hostString.toUpperCase();
    infoMsg.concat(F("<br>Chip short id:   "));infoMsg.concat(WIFI_CHIPID_PREFIX); infoMsg.concat(hostString);

    infoMsg.concat(F("<br>Flash real size: ")); infoMsg.concat(realSize);

    infoMsg.concat(F("<br>Flash ide  size: ")); infoMsg.concat(ideSize);
    infoMsg.concat(F("<br>Flash ide speed: ")); infoMsg.concat(ESP.getFlashChipSpeed());
    infoMsg.concat(F("<br>Flash ide mode:  ")); infoMsg.concat((ideMode == FM_QIO ? "QIO" : ideMode == FM_QOUT ? "QOUT" : ideMode == FM_DIO ? "DIO" : ideMode == FM_DOUT ? "DOUT" : "UNKNOWN"));
    if(ideSize != realSize)
    {
        infoMsg.concat(F("<br>Flash Chip configuration wrong!\r\n"));
    }
    else
    {
        infoMsg.concat(F("<br>Flash Chip configuration ok.\r\n"));
    }
#if defined(ESP8266)
    srv_return_msg.concat(F("<br> ESP8266 Chip id = ")); srv_return_msg.concat(ESP.getChipId());
#endif
    infoMsg.concat(F("<br><br>"));
    if (LITTLEFS_BEGIN_FUNC_CALL) {
        infoMsg.concat(F("<br>LittleFS mounted OK"));
#if defined(ESP8266)
        FSInfo fsi;
        if (LittleFS.info(fsi)) {
            srv_return_msg.concat(F("<br>LittleFS blocksize = ")); srv_return_msg.concat(fsi.blockSize); 
            srv_return_msg.concat(F("<br>LittleFS maxOpenFiles = ")); srv_return_msg.concat(fsi.maxOpenFiles); 
            srv_return_msg.concat(F("<br>LittleFS maxPathLength = ")); srv_return_msg.concat(fsi.maxPathLength); 
            srv_return_msg.concat(F("<br>LittleFS pageSize = ")); srv_return_msg.concat(fsi.pageSize); 
            srv_return_msg.concat(F("<br>LittleFS totalBytes = ")); srv_return_msg.concat(fsi.totalBytes); 
            srv_return_msg.concat(F("<br>LittleFS usedBytes = ")); srv_return_msg.concat(fsi.usedBytes); 
        }
        else
#elif defined(ESP32)
        infoMsg.concat(F("<br>LittleFS totalBytes = ")); infoMsg.concat(LittleFS.totalBytes());
        infoMsg.concat(F("<br>LittleFS usedBytes = ")); infoMsg.concat(LittleFS.usedBytes()); 
#else
            srv_return_msg.concat(F("<br>LittleFS info not implemented"));
#endif

        infoMsg.concat("<br><br>Files:<br>");
        
        //LittleFS_ext::listDir(DEBUG_UART,"/", 0);
        LittleFS_ext::listDir(infoMsg, true, "/", 0);
    }
    else
        infoMsg.concat(F("<br>LittleFS Fail to mount"));

    infoMsg.concat(F("</body></html>"));
    return infoMsg;
}
}