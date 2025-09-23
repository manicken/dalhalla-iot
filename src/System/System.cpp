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

#include "System.h"
#include <Arduino.h>
#include <ArduinoOTA.h>
#include "Drivers/HearbeatLed.h" // this should not be here in final version (should only be accessible through HAL interface)

#if defined(ESP8266)
#include <ESP8266mDNS.h>
#elif defined(ESP32)
#include <SD_MMC.h>
#include <ESPmDNS.h>
#endif

#include "../Support/ConstantStrings.h"

#include "System/MainConfig.h"

namespace System {

#if defined(ESP32)

#define INIT_SDMMC_PRINT_INFO
#define INIT_SDMMC_PRINT_DIR
    bool InitSD_MMC()
    {
        pinMode(23, OUTPUT); // output
        digitalWrite(23, HIGH); // enable pullup on IO2(SD_D0), IO12(SD_D2)
        pinMode(2, INPUT); // input
        if (digitalRead(2) == 0) return false; // no pullup connected to GPIO2 from GPIO23
        delay(10);
        log_e("SD-card initialialize...");

        if (SD_MMC.begin("/sdcard", false, false, 20000)) {
            DEBUG_UART.println("SD-card initialized OK");
#if defined(INIT_SDMMC_PRINT_INFO)
            DEBUG_UART.print("SD card size:"); DEBUG_UART.println(SD_MMC.cardSize());
            DEBUG_UART.print("SD card type:"); 
            if (SD_MMC.cardType() == sdcard_type_t::CARD_SD) DEBUG_UART.println("CARD_SD");
            else if (SD_MMC.cardType() == sdcard_type_t::CARD_MMC) DEBUG_UART.println("CARD_MMC");
            else if (SD_MMC.cardType() == sdcard_type_t::CARD_NONE) DEBUG_UART.println("CARD_NONE");
            else if (SD_MMC.cardType() == sdcard_type_t::CARD_SDHC) DEBUG_UART.println("CARD_SDHC");
            else if (SD_MMC.cardType() == sdcard_type_t::CARD_UNKNOWN) DEBUG_UART.println("CARD_UNKNOWN");

            DEBUG_UART.print("SD card totalBytes:"); DEBUG_UART.println(SD_MMC.totalBytes());
            DEBUG_UART.print("SD card usedBytes:"); DEBUG_UART.println(SD_MMC.usedBytes());
#endif
#if defined(INIT_SDMMC_PRINT_DIR)
            FS* fileSystem = &SD_MMC;
            File root = fileSystem->open("/");

            File file;
            while (file = root.openNextFile())
            {
                DEBUG_UART.print("Name:"); DEBUG_UART.print(file.name());
                DEBUG_UART.print(", Size:"); DEBUG_UART.print(file.size());
                DEBUG_UART.print(", Dir:"); DEBUG_UART.print(file.isDirectory()?"true":"false");
                DEBUG_UART.println();
            }
#endif
            return true;
        }
        else
        {
            log_e("could not initialize/find any connected sd-card.");
            return false;
        }
    }
#endif
#if defined(ESP32)
    void Start_MDNS()
    {
        DEBUG_UART.println("\n\n***** STARTING mDNS service ********");
        if (MDNS.begin(MainConfig::mDNS_name.c_str())) {
            mdns_instance_name_set("ESP32 development board");
            MDNS.addService("http", "tcp", 80);
            
            if (mdns_service_add("_esp32devices", "http", "tcp", 80, NULL, 0) != ESP_OK)
                DEBUG_UART.println("Failed adding service view");
        // MDNS.addServiceTxt("http", "tcp", "path", "/");
            DEBUG_UART.println("MDNS started with name:" + MainConfig::mDNS_name);
        }
        else {
            DEBUG_UART.println("MDNS could not start");
        }
        DEBUG_UART.println("\n");
    }
#endif

    void failsafeLoop()
    {
        // blink rapid to alert a crash
        HeartbeatLed::HEARTBEATLED_OFF_INTERVAL = 300;
        HeartbeatLed::HEARTBEATLED_ON_INTERVAL = 300;
        //connect_to_wifi();
#ifdef WIFI_MANAGER_WRAPPER_H_
        WiFiManager_Handler(display);
#endif
        
        DEBUG_UART.begin(115200);
        DEBUG_UART.println();
        DEBUG_UART.println(F("************************************"));
        DEBUG_UART.println(F("* Now entering failsafe OTA loop.. *"));
        DEBUG_UART.println(F("************************************"));

        AsyncWebServer* asyncServer = new AsyncWebServer(80);
        asyncServer->on("/reset", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send(200, "text/plain", "The system will now reset and luckily go into normal mode.");
            // Small delay to ensure the response is sent before restarting
            delay(100); // NOT blocking in this context, short enough to work
#if defined(ESP32)
            esp_restart();  // Software reset
#elif defined(ESP8266)
            ESP.restart();
#endif
        });
        asyncServer->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send(200, "text/plain", "The system is in OTA failsafe loop.");
        });
        asyncServer->begin();

        while (1)
        {
            ArduinoOTA.handle();
            HeartbeatLed::task();
        }
    }

    void initWebServerHandlers(AsyncWebServer& webserver)
    {
        webserver.on(MAIN_URLS_FORMAT_LITTLE_FS, [](AsyncWebServerRequest* req) {
            
            if (LittleFS.format()) {
                if (!LITTLEFS_BEGIN_FUNC_CALL) {
                    req->send(500, "text/html", "Format OK, but mount failed");
                    return;
                }
                req->send(200, "text/html", "Format OK and mounted");
            } else {
                req->send(500, "text/html", "Format failed");
            }
        });
        webserver.on(MAIN_URLS_MKDIR, [](AsyncWebServerRequest* req) {
            if (!req->hasArg("dir")) { req->send(200,"text/html", "Error: dir argument missing"); }
            else
            {
                String path = req->arg("dir");
                
                if (LittleFS.mkdir(path.c_str()) == false) {
                    req->send(200,"text/html", "Error: could not create dir:" + path);
                }
                else
                {
                    req->send(200,"text/html", "create new dir OK");
                }

            }

        });
#if defined(ESP32)
        webserver.on("/sdcard_listfiles", [](AsyncWebServerRequest* req) {
            
            //if (SD_MMC.begin("/sdcard", true, false, 20000)) {
                File root = SD_MMC.open("/");
                if (!root) {
                    req->send(200, CONSTSTR::htmlContentType_TextPlain, "error while open sd card again");
                    return;
                }

                File file;
                String ret;
                while (file = root.openNextFile())
                {
                    ret.concat("Name:"); ret.concat(file.name());
                    ret.concat(", Size:"); ret.concat(file.size());
                    ret.concat(", Dir:"); ret.concat(file.isDirectory()?"true":"false");
                    ret.concat("\n");
                }
                req->send(200, CONSTSTR::htmlContentType_TextPlain, ret.c_str());
        // }else {webserver.send(200, CONSTSTR::htmlContentType_TextPlain, "could not open sd card a second time");}
        });
#endif
        webserver.on("/crashTest", [](AsyncWebServerRequest* req) {
            req->send(200, CONSTSTR::htmlContentType_TextPlain, "The system will now crash!!!, and luckily go into failsafe OTA upload mode.");
            int *ptr = nullptr; // Null pointer
            *ptr = 42;          // Dereference the null pointer (causes a crash)
        });

        /*
        webserver.onNotFound([](AsyncWebServerRequest* req) {                              // If the client requests any URI
            String uri = req->uri();
            DEBUG_UART.println("onNotFound - hostHeader:" + req->hostHeader());
            //bool isDir = false;
            //DEBUG_UART.println("webserver on not found:" + uri);
            if (uri.startsWith("/LittleFS") == false && uri.startsWith("/sdcard") == false)
            {
#if defined(ESP32)
                File fileToCheck = LittleFS.open(uri);
#elif defined(ESP8266)
                File fileToCheck = LittleFS.open(uri, "r");
#endif
                if (!fileToCheck) {
                    webserver.send(404, CONSTSTR::htmlContentType_TextPlain, F("404: Not Found")); // otherwise, respond with a 404 (Not Found) error
                    return;
                }

                if (fileToCheck.isDirectory()) // if it's a folder, try to find index.htm or index.html
                {
                    if (LittleFS.exists(uri + "/index.html"))
                        uri += "/index.html";
                    else if (LittleFS.exists(uri + "/index.htm"))
                        uri += "/index.htm";
                }
                fileToCheck.close();
                uri = "/LittleFS" + uri; // default to LittleFS
            }
#ifdef FSBROWSER_SYNCED_WS_H_
            if (!FSBrowser::handleFileRead(uri))                  // send it if it exists
                webserver.send(404, CONSTSTR::htmlContentType_TextPlain, F("404: Not Found")); // otherwise, respond with a 404 (Not Found) error
#endif
        });
        */
    }
}