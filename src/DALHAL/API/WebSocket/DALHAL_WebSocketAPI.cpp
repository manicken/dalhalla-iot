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
#if defined(ESP32)
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#endif

#include <DALHAL/API/DALHAL_CommandExecutor.h>

#include <DALHAL/Support/DALHAL_Logger.h>

#include <LittleFS.h>
//#include "FrontendFiles/DALHAL_WebSocketAPI_httpFile.h"

#if defined(ESP8266) || defined(ESP32)
#include <System/System.h> //  defines DALHAL_SYSTEM_H_
#endif

#include "FrontendFiles/DALHAL_VirtualFile.h"
#include "FrontendFiles/DALHAL_FileList.h"

namespace DALHAL {

    AsyncWebServer* WebSocketAPI::asyncWebserver = nullptr;
    AsyncWebSocket* WebSocketAPI::asyncWebSocket = nullptr;
    bool WebSocketAPI::failsafeMode = false;

    void WebSocketAPI::onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
    {
        //Serial.println("onWsEvent");

        switch (type) {

        case WS_EVT_CONNECT:
            Serial.print(F("WS client #"));
            Serial.print(client->id());
            Serial.print(F(" connected from "));
            Serial.print(client->remoteIP().toString().c_str());
            Serial.println();
            
            if (failsafeMode) {
                client->text(F("failsafe/recovery mode is active"));
            } else {
                client->text(F("normal mode"));
            }
            break;

        case WS_EVT_DISCONNECT:
            Serial.print(F("WS client #"));
            Serial.print(client->id());
            Serial.print(F(" disconnected"));
            
            break;

        case WS_EVT_DATA: {
            AwsFrameInfo *info = (AwsFrameInfo*)arg;

            if (!info->final || info->index != 0 || info->len != len) {
                Serial.println(F("WS EVT_DATA rx error"));
                return;
            }
            if (info->opcode != WS_TEXT) {
                Serial.println(F("info->opcode != WS_TEXT"));
                return;
            }

            std::string cmd((char*)data, len);
            //Serial.printf("WS RX: %s\n", cmd.c_str());
#if defined(DALHAL_SYSTEM_H_)
            // absolute failsafe command as WS run througth AsyncWebServer and runs in the BG it's mostly allways available
            ZeroCopyString zcCmd((char*)data, len);
            
            if (zcCmd.EqualsIC(F("system/EnterRecoveryMode"))) {
                System::EnterRecoveryMode();
            }
#endif
            //client->text("ACK");
            uint32_t clientId = client->id();

            CommandExecutor_LOCK_QUEUE();
            CommandExecutor::g_pending.push({
                std::move(cmd),
                [clientId](const ZeroCopyString& body, CmdCbType type) -> bool {
                    
                    AsyncWebSocketClient* c = asyncWebSocket->client(clientId);

                    if (!c) {
                        GlobalLogger.Error(F("client gone while WebSocket write"));
                        Serial.println(F("client gone while WebSocket write"));
                        return false;                 // client gone
                    }
                    //bool abortSend = false;
                    uint32_t retryCount = 0;
                    while (!c->canSend()) {
                        delay(1);
                        retryCount++;
                        if (retryCount > 10000) {
                            //abortSend = true;
                            GlobalLogger.Error(F("client could not write WebSocket data"));
                            Serial.println(F("client could not write WebSocket data"));
                            return false;
                        }
                    }      // TCP buffer full / closing
                    if (type == CmdCbType::Control) {
                        // send control as text
                        c->text(body.start, body.Length());
                    } else if (type == CmdCbType::Data) {
                        // send data as binary to make it separate from control
                        c->binary(body.start, body.Length());
                    }                    
                    return true;
                }
            });
            CommandExecutor_UNLOCK_QUEUE();

            break;
        }

        case WS_EVT_ERROR:
            Serial.println(F("WS error"));
            break;

        case WS_EVT_PONG:
            Serial.println(F("WS_EVT_PONG"));
            break;

        default:
            Serial.println(F("WS unknown type"));
            break;
        }
    }

    /*void WebSocketAPI::GetRootPage_Handler(AsyncWebServerRequest* request) {

        const __FlashStringHelper* path = F("/api/index.html");

        if (LittleFS.exists(path)) {
            request->send(LittleFS, path, F("text/html"));
            return;
        }

        request->send_P(200, F("text/html"), HTML_WS_CONSOLE);
    }*/

    void WebSocketAPI::GetAnyFile_Handler(AsyncWebServerRequest* request) {
        const String& url = request->url();
        const bool isRoot = url.length() == 1;

        String path;
        // avoid heap fragmentation
        path.reserve(5 + (isRoot ? sizeof("/index.html") : url.length()));
        
        path = F("/api");
        if (isRoot) {
            path += F("/index.html");
        } else {
            path += url;
        }

        if (LittleFS.exists(path)) {
            request->send(LittleFS, path);
        } else {
            
            ZeroCopyString zcFilePath = url.c_str();
            if (zcFilePath.StartsWith('/')) {
                zcFilePath.start++;
            }
            const ApiVirtualFile* vFile = GetApiVirtualFile(zcFilePath);
            if (vFile == nullptr) {
                request->send(404, F("text/plain"), F("file not found"));
                return;
            }
            request->send_P(200, vFile->mime, vFile->data, vFile->size);
        }
    }

    void WebSocketAPI::setup(bool failsafeMode) {
        WebSocketAPI::failsafeMode = failsafeMode;
        if (asyncWebserver != nullptr) {
            // just restart server
            if (asyncWebSocket != nullptr) {
                asyncWebSocket->cleanupClients();
            }
            asyncWebserver->end();
            asyncWebserver->begin();
            return;
        }
        asyncWebserver = new AsyncWebServer(82);
        asyncWebSocket = new AsyncWebSocket("/ws");
        asyncWebSocket->onEvent(onWsEvent);
        asyncWebserver->addHandler(asyncWebSocket);
        //asyncWebserver->on("/", GetRootPage_Handler);
        asyncWebserver->onNotFound(GetAnyFile_Handler);
        asyncWebserver->begin();
        asyncWebSocket->enable(true);

    }


    bool WebSocketAPI::Broadcast(const char* msg, size_t len, CmdCbType type) {
        if (asyncWebSocket->availableForWriteAll()) {
            if (type == CmdCbType::Control) {
                asyncWebSocket->textAll(msg, len);
                return true;
            } else if (type == CmdCbType::Data) {
                // send data as binary to make it separate from control
                asyncWebSocket->binaryAll(msg, len);
                return true;
            }
        }
        return false;
    }

    bool WebSocketAPI::Broadcast(std::string &msg, CmdCbType type) {
        return Broadcast(msg.c_str(), msg.length(), type);
    }
    bool WebSocketAPI::Broadcast(const char* msg, CmdCbType type) {
        return Broadcast(msg, strlen(msg), type);
    }
    bool WebSocketAPI::Broadcast(const ZeroCopyString& zcStr, CmdCbType type) {
        return Broadcast(zcStr.start, zcStr.Length(), type);
    }
    bool WebSocketAPI::BroadcastCb(const ZeroCopyString& zcStr, CmdCbType type) {
        return Broadcast(zcStr.start, zcStr.Length(), type);
    }
    bool WebSocketAPI::Broadcast(const char* source, const char* msg, CmdCbType type) {
        if (asyncWebSocket->availableForWriteAll()) {
            std::string msgStr;
            msgStr.reserve(strlen(source) + strlen(msg) + 2);
            msgStr.append(source);
            msgStr.append(msg);
            return Broadcast(msgStr.c_str(), msgStr.length(), type);
        } else {
            return false;
        }
    }

}
#endif