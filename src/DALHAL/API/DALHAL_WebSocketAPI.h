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

#include <Arduino.h>
//#include <ArduinoJson.h>
#include <stdlib.h>
#include <LittleFS.h>

#include <DALHAL/Core/Types/DALHAL_ZeroCopyString.h>
#include <DALHAL/API/DALHAL_CommandCallback.h>

#if defined(ESP32) || defined(ESP8266)
  #include <Support/LittleFS_ext.h>
  #include <WiFiClient.h>
#else
  #include <LittleFS_ext.h>
#endif

#include <ESPAsyncWebServer.h> // have a stub wrapper for this



#define DALHAL_WEBSOCKET_API_PORT 82

namespace DALHAL {
    class WebSocketAPI {
    private:
        static AsyncWebServer* asyncWebserver;
        static AsyncWebSocket* asyncWebSocket;
        static void GetRootPage_Handler(AsyncWebServerRequest* request);
        static void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
        static bool failsafeMode;
    public:
        static void setup(bool failsafeMode = false);
        inline static void loop() { 
          asyncWebSocket->cleanupClients();
        }

        static bool Broadcast(const char* msg, size_t len, CmdCbType type = CmdCbType::Control);
        static bool Broadcast(std::string &msg, CmdCbType type = CmdCbType::Control);
        static bool Broadcast(const char* msg, CmdCbType type = CmdCbType::Control);
        static bool Broadcast(const ZeroCopyString& zcStr, CmdCbType type = CmdCbType::Control);
        static bool BroadcastCb(const ZeroCopyString& zcStr, CmdCbType type);
        /** can be used to combine two messages */ 
        static bool Broadcast(const char* source, const char* msg, CmdCbType type = CmdCbType::Control);

    };
}