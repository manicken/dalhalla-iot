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
// OTA, a minimal HTTP page on port 80 and the WebSocket API in failsafe mode. Restarts after 10 minutes.

#include "failsafe.h"

#include <Arduino.h>
#include "esp_http_server.h"

#include <ArduinoOTA.h>
#include <DALHAL/Drivers/HearbeatLed.h>
#include <DALHAL/API/DALHAL_CommandExecutor.h>
#include <DALHAL/API/WebSocket/DALHAL_WebSocketAPI_Backend.h>
#include <System/Network/Network.h>
#include <System/Platform/Platform.h>

#define FAILSAFE_TIMEOUT_MS (10UL * 60UL * 1000UL)

static esp_err_t RootHandler(httpd_req_t* req) {
    httpd_resp_set_type(req, "text/plain");
    return httpd_resp_sendstr(req, "Device is in FAILSAFE mode.\nUse OTA or /reset");
}

static esp_err_t ResetHandler(httpd_req_t* req) {
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_sendstr(req, "Restarting...");
    delay(200);
    Platform::Restart();
}

static void StartHttpServer() {
    httpd_handle_t server = nullptr;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;
    config.ctrl_port = 32769; // the WebSocketAPI server has its own ctrl port
    if (httpd_start(&server, &config) != ESP_OK) {
        printf("Failsafe HTTP server start failed\r\n");
        return;
    }
    httpd_uri_t root = {};
    root.uri = "/";
    root.method = HTTP_GET;
    root.handler = RootHandler;
    httpd_register_uri_handler(server, &root);

    httpd_uri_t reset = {};
    reset.uri = "/reset";
    reset.method = HTTP_GET;
    reset.handler = ResetHandler;
    httpd_register_uri_handler(server, &reset);
    printf("\r\nFailsafe HTTP server started\r\n");
}

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

    StartHttpServer();
    DALHAL::WebSocketAPI::setup(/*failsafeMode*/ true);

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
