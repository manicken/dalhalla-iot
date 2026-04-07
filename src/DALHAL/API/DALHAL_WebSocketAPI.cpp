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
#if defined(ESP8266) || defined(ESP32)



#include "DALHAL_WebSocketAPI.h"

#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"

#include "DALHAL_CommandExecutor.h"

#include "DALHAL_WebSocketAPI_httpFile.h"

namespace DALHAL {

    AsyncWebServer* WebSocketAPI::asyncWebserver = nullptr;
    AsyncWebSocket* WebSocketAPI::asyncWebSocket = nullptr;

    void WebSocketAPI::onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
    {
        //Serial.println("onWsEvent");

        switch (type) {

        case WS_EVT_CONNECT:
            Serial.printf("WS client #%u connected from %s\r\n",
                        client->id(),
                        client->remoteIP().toString().c_str());
            break;

        case WS_EVT_DISCONNECT:
            Serial.printf("WS client #%u disconnected\r\n", client->id());
            break;

        case WS_EVT_DATA: {
            AwsFrameInfo *info = (AwsFrameInfo*)arg;

            if (!info->final || info->index != 0 || info->len != len) {
                Serial.println("WS EVT_DATA rx error");
                return;
            }
            if (info->opcode != WS_TEXT) {
                Serial.println("info->opcode != WS_TEXT");
                return;
            }

            std::string cmd((char*)data, len);
            //Serial.printf("WS RX: %s\n", cmd.c_str());

            //client->text("ACK");
            uint32_t clientId = client->id();
            CommandExecutor_LOCK_QUEUE();
            CommandExecutor::g_pending.push({
                cmd,
                [clientId](const std::string& body) {
                    
                    AsyncWebSocketClient* c = asyncWebSocket->client(clientId);

                    if (!c) return;                 // client gone
                    if (!c->canSend()) return;      // TCP buffer full / closing

                    c->text(body.c_str());
                    
                }
            });
            CommandExecutor_UNLOCK_QUEUE();

            break;
        }

        case WS_EVT_ERROR:
            Serial.println("WS error");
            break;

        case WS_EVT_PONG:
            Serial.println("WS_EVT_PONG");
            break;

        default:
            Serial.println("WS unknown type");
            break;
        }
    }

    void WebSocketAPI::GetRootPage_Handler(AsyncWebServerRequest* request) {
        request->send(200, "text/html", HTML_WS_CONSOLE);
    }

    void WebSocketAPI::setup() {
        asyncWebserver = new AsyncWebServer(82);
        asyncWebSocket = new AsyncWebSocket("/ws");
        asyncWebSocket->onEvent(onWsEvent);
        asyncWebserver->addHandler(asyncWebSocket);
        asyncWebserver->on("/", GetRootPage_Handler);
        asyncWebserver->begin();
        asyncWebSocket->enable(true);
    }

    void WebSocketAPI::Broadcast(std::string &msg) {
        if (asyncWebSocket->availableForWriteAll()) {
            asyncWebSocket->textAll(msg.c_str());
        }
    }
    void WebSocketAPI::Broadcast(const char* msg) {
        if (asyncWebSocket->availableForWriteAll()) {
            asyncWebSocket->textAll(msg);
        }
    }
    void WebSocketAPI::Broadcast(const char* source, const char* msg) {
        if (asyncWebSocket->availableForWriteAll()) {
            std::string msgStr;
            msgStr.reserve(strlen(source) + strlen(msg) + 2);
            msgStr.append(source);
            msgStr.append(msg);
            asyncWebSocket->textAll(msg);
        }
    }

}
#endif