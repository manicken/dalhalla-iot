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
#if defined(ESP32) || defined(ESP8266)

#include "System.h"
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <DALHAL/Drivers/HearbeatLed.h> // this should not be here in final version (should only be accessible through HAL interface)

#if defined(ESP8266)
#include <ESP8266mDNS.h>
#elif defined(ESP32) && !defined(seeed_xiao_esp32c3)
#include <SD_MMC.h>
#include <ESPmDNS.h>
#endif

#include "OTA.h"
#include "HttpServer.h"
#include "FS_Http.h"
#include <System/Network/Network.h>
#include <System/Platform/Platform.h>

#include <DALHAL/API/DALHAL_API.h>
#include <DALHAL/API/DALHAL_CommandExecutor.h>

namespace System {

    void EnterRecoveryMode()
    {
        Platform::Abort("******** FORCED_RECOVERY ********");
    }

    void Setup()
    {
        // needed before all access to the internal flash filesystem (cfg.json, scripts, /fs/... routes)
        if (!LITTLEFS_BEGIN_FUNC_CALL) Serial.println(F("LittleFS mount failed"));
    }

#if defined(ESP32) && !defined(esp32c3)

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

    void failsafeLoop_API_exec_cmd() {
         while (true) {
            CommandExecutor_LOCK_QUEUE();
            if (DALHAL::CommandExecutor::g_pending.empty()) {
                CommandExecutor_UNLOCK_QUEUE();
                break;
            }
            DALHAL::PendingRequest pr = std::move(DALHAL::CommandExecutor::g_pending.front());
            DALHAL::CommandExecutor::g_pending.pop();
            CommandExecutor_UNLOCK_QUEUE();

            DALHAL::ZeroCopyString zcCmd(pr.command.c_str());
            DALHAL::CommandExecutor::execute(zcCmd, pr.cb);
        }
    }

    void failsafeLoop()
    {
        // Fast blink on crash
        HeartbeatLed::setup(500, 500);
        
        Serial.begin(115200);
        delay(500);  // Give Serial time to initialize
        Serial.println(F("\r\n\n"));
        Serial.println(F("======================================"));
        Serial.println(F("      FAILSAFE MODE ACTIVATED"));
        Serial.println(F("======================================\r\n"));

        // --- STEP 0+1: bring up all network links in failsafe mode (WiFi: failsafe AP + STA with stored credentials) ---
        Network::BeginFailsafe();

        // --- STEP 2: Setup OTA ---
        Serial.println(F("\n[OTA] Configuring ArduinoOTA..."));
        ArduinoOTA.onStart([](){ Serial.println(F("OTA Start")); });
        ArduinoOTA.onEnd([](){ Serial.println(F("OTA End")); });
        ArduinoOTA.onError([](ota_error_t err){
            Serial.print(F("OTA Error ")); Serial.println(err);
        });
        ArduinoOTA.begin();

        // --- STEP 3: the web server (port 80): frontend + /ws in failsafe mode, /fs/... to fix a broken cfg, /reset ---
        AsyncWebServer& server = HttpServer::Get();

        server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *req){
            String type = F("text/plain");
            String msg = F("Restarting...");
            req->send(200, type, msg);
            delay(200);
            Platform::Restart();
        });

        if (!LITTLEFS_BEGIN_FUNC_CALL) Serial.println(F("LittleFS mount failed")); // for /fs/..., DALHAL does not read it here
        FS_Http::setup(server);
        DALHAL::WebSocketAPI::setup(/*failsafemode*/true);

        server.begin();
        Serial.println(F("\r\nFailsafe HTTP server started"));
        unsigned long long failsafeLoopTimeoutMs = 1000*60*10; // 10 Min timeout to avoid stuck in this state in case of a error
        unsigned long long failsafeLoopTimeoutMs_Start = millis();
        unsigned long long failsafeLoopTimeoutMs_MaxEnd = failsafeLoopTimeoutMs_Start + failsafeLoopTimeoutMs;
        // --- STEP 4: FAILSAFE LOOP ---
        while (true) {
            Network::LoopFailsafe();
            ArduinoOTA.handle();    // Required
            HeartbeatLed::task();   // Non-blocking

            if (millis() >= failsafeLoopTimeoutMs_MaxEnd) {
                Serial.println(F("\r\n"));
                Serial.println(F("======================================"));
                Serial.println(F("  FAILSAFE TIMEOUT REACHED (10 MIN)"));
                Serial.println(F("  Restarting device..."));
                Serial.println(F("======================================\r\n"));
                Platform::Restart();
            }
            failsafeLoop_API_exec_cmd();           
            delay(1);              // Prevent WDT reset
        }
    }


    void initWebServerHandlers(AsyncWebServer& webserver)
    {
        FS_Http::setup(webserver); // /fs/... for the Files tab of the frontend
        webserver.on("/reset", [](AsyncWebServerRequest* req) {
            req->send(200, "text/html", "system will now reset");
            Platform::Restart();
            
        });
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
        /*webserver.on("/crashTest", [](AsyncWebServerRequest* req) {
            req->send_P(200, F("text/plain"), F("The system will now crash!!!, and luckily go into failsafe OTA upload mode."));
            int *ptr = nullptr; // Null pointer
            *ptr = 42;          // Dereference the null pointer (causes a crash)
        });*/
        /*webserver.on("/enableOTA", [](AsyncWebServerRequest* req) {
            req->send(200, "text/plain", "ArduinoOTA is now guarranteed to work");
            ArduinoOTA.begin();
        });*/
    }
}

#endif