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

// Failsafe mode after a crash, the same as System::failsafeLoop() of the Arduino target (src/System/System.cpp):
// DALHAL itself is not started (it is what crashed), only the ways in: network (failsafe AP + STA),
// OTA, the HttpServer on port 80 with the WebSocket API in failsafe mode, the built in frontend, /fs/... file
// transfer (to fix a broken cfg.json) and /reset. Restarts after 10 minutes.

#include "failsafe.h"

#include <Arduino.h>
#include <LittleFS.h>

#include <ArduinoOTA.h>
#include <FS_Http.h>
#include <DALHAL/Drivers/HearbeatLed.h>
#include <DALHAL/API/DALHAL_CommandExecutor.h>
#include <DALHAL/API/WebSocket/DALHAL_WebSocketAPI_Backend.h>
#include <System/Network/Network.h>
#include <System/Platform/Platform.h>

#define FAILSAFE_TIMEOUT_MS (10UL * 60UL * 1000UL)

/** only the command queue, no devices or scripts */
static void ExecPendingCommands() {
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

void FailsafeLoop() {
    HeartbeatLed::setup(500, 500); // fast blink on crash

    printf("\r\n\n======================================\r\n");
    printf("      FAILSAFE MODE ACTIVATED\r\n");
    printf("======================================\r\n\r\n");

    Network::BeginFailsafe(); // WiFi: failsafe AP + STA with the stored credentials

    ArduinoOTA.onError([](ota_error_t err) { printf("OTA Error %d\r\n", (int)err); });
    ArduinoOTA.begin();

    if (!LittleFS.begin(false)) printf("LittleFS mount failed\r\n"); // for /fs/..., DALHAL does not read it here

    DALHAL::WebSocketAPI::setup(/*failsafeMode*/ true); // starts the HttpServer (port 80, /reset)
    FS_Http::setup();
    printf("\r\nFailsafe HTTP server started\r\n");

    const unsigned long start = millis();
    while (true) {
        Network::LoopFailsafe();
        ArduinoOTA.handle();
        HeartbeatLed::task();
        ExecPendingCommands();

        if (millis() - start >= FAILSAFE_TIMEOUT_MS) {
            printf("\r\n======================================\r\n");
            printf("  FAILSAFE TIMEOUT REACHED (10 MIN)\r\n");
            printf("  Restarting device...\r\n");
            printf("======================================\r\n\r\n");
            Platform::Restart();
        }
        vTaskDelay(1);
    }
}
