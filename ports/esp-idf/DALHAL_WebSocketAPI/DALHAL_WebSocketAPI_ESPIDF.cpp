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

#include "DALHAL_WebSocketAPI_ESPIDF.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>

#include "esp_http_server.h"
#include <HttpServer.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <LittleFS.h>
#include <Support/mimetable.h>
#include <DALHAL/API/DALHAL_CommandExecutor.h>
#include <DALHAL/API/WebSocket/FrontendFiles/DALHAL_VirtualFile.h>
#include <DALHAL/API/WebSocket/FrontendFiles/DALHAL_FileList.h>
#include <System/Platform/Platform.h>

// longest WebSocket text frame accepted (a command line, a cfg upload is chunked by the frontend)
#define DALHAL_WS_MAX_FRAME_LENGTH (16 * 1024)

namespace DALHAL {

    static httpd_handle_t server = nullptr;
    static bool failsafe = false;
    /** the httpd task, httpd_ws_send_data() must not be called from it (it waits for that task) */
    static TaskHandle_t httpdTask = nullptr;

    static httpd_ws_type_t FrameType(CmdCbType type) {
        return type == CmdCbType::Data ? HTTPD_WS_TYPE_BINARY : HTTPD_WS_TYPE_TEXT;
    }

    /** sends one frame to one client, from any task */
    static bool SendFrame(int fd, const char* data, size_t len, httpd_ws_type_t type) {
        if (server == nullptr) return false;
        httpd_ws_frame_t frame = {};
        frame.final = true;
        frame.type = type;
        frame.payload = reinterpret_cast<uint8_t*>(const_cast<char*>(data));
        frame.len = len;
        esp_err_t err;
        if (xTaskGetCurrentTaskHandle() == httpdTask) {
            err = httpd_ws_send_frame_async(server, fd, &frame); // already in the httpd task
        } else {
            err = httpd_ws_send_data(server, fd, &frame);        // queued to the httpd task, waits until sent
        }
        return err == ESP_OK;
    }

    // ---------------- WebSocket ----------------

    static void SendGreeting(void* arg) {
        int fd = (int)(intptr_t)arg;
        const char* msg = failsafe ? "failsafe/recovery mode is active" : "normal mode";
        SendFrame(fd, msg, strlen(msg), HTTPD_WS_TYPE_TEXT);
    }

    static esp_err_t WsHandler(httpd_req_t* req) {
        httpdTask = xTaskGetCurrentTaskHandle();
        int fd = httpd_req_to_sockfd(req);

        if (req->method == HTTP_GET) { // the handshake, the socket is a WebSocket after this returns
            printf("WS client #%d connected\r\n", fd);
            httpd_queue_work(req->handle, SendGreeting, (void*)(intptr_t)fd);
            return ESP_OK;
        }

        httpd_ws_frame_t frame = {};
        esp_err_t err = httpd_ws_recv_frame(req, &frame, 0); // length only
        if (err != ESP_OK) return err;
        if (frame.type != HTTPD_WS_TYPE_TEXT) {
            printf("WS frame is not text, ignored\r\n");
            return ESP_OK;
        }
        if (frame.len == 0) return ESP_OK;
        if (frame.len > DALHAL_WS_MAX_FRAME_LENGTH) {
            printf("WS frame too long (%u), ignored\r\n", (unsigned)frame.len);
            return ESP_OK;
        }
        std::string cmd(frame.len, '\0');
        frame.payload = reinterpret_cast<uint8_t*>(&cmd[0]);
        err = httpd_ws_recv_frame(req, &frame, frame.len);
        if (err != ESP_OK) return err;
        cmd.resize(frame.len);

        // absolute failsafe command, handled here as the command queue may be the thing that hangs
        if (ZeroCopyString(cmd.c_str()).EqualsIC("system/EnterRecoveryMode")) {
            Platform::Abort("******** FORCED_RECOVERY ********");
        }

        // executed in the DALHAL loop (DALHAL::loop), the answer goes back to this client
        CommandExecutor_LOCK_QUEUE();
        CommandExecutor::g_pending.push({
            std::move(cmd),
            [fd](const ZeroCopyString& body, CmdCbType type) -> bool {
                if (httpd_ws_get_fd_info(server, fd) != HTTPD_WS_CLIENT_WEBSOCKET) {
                    printf("WS client gone while WebSocket write\r\n");
                    return false;
                }
                return SendFrame(fd, body.start, body.Length(), FrameType(type));
            }
        });
        CommandExecutor_UNLOCK_QUEUE();
        return ESP_OK;
    }

    // ---------------- files ----------------

    /** sends a LittleFS file, false when there is no such file (nothing sent then) */
    static bool SendLittleFSFile(httpd_req_t* req, const std::string& path) {
        File f = LittleFS.open(path.c_str(), "r");
        if (!f || f.isDirectory()) return false;
        httpd_resp_set_type(req, mime::getContentType(path));
        char buf[1024];
        size_t n;
        while ((n = f.readBytes(buf, sizeof(buf))) > 0) {
            if (httpd_resp_send_chunk(req, buf, n) != ESP_OK) break; // client gone
        }
        httpd_resp_send_chunk(req, nullptr, 0);
        return true;
    }

    static esp_err_t FileHandler(httpd_req_t* req) {
        // the path without query
        std::string url(req->uri);
        size_t q = url.find('?');
        if (q != std::string::npos) url.resize(q);
        const bool isRoot = url == "/";

        // a file in LittleFS /api/ overrides the built in frontend, as on the Arduino version
        if (SendLittleFSFile(req, std::string("/api") + (isRoot ? "/index.html" : url))) return ESP_OK;

        ZeroCopyString zcFilePath(isRoot ? "index.html" : url.c_str() + 1);
        const ApiVirtualFile* vFile = GetApiVirtualFile(zcFilePath);
        if (vFile != nullptr) {
            httpd_resp_set_type(req, vFile->mime);
            if (vFile->gzip) httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
            return httpd_resp_send(req, reinterpret_cast<const char*>(vFile->data), vFile->size);
        }

        // any other file in LittleFS (own pages, as on the Arduino port 80 server), a folder gives its index.htm
        if (!isRoot && SendLittleFSFile(req, url)) return ESP_OK;
        if (!isRoot) {
            std::string index = url + (url.back() == '/' ? "index.htm" : "/index.htm");
            if (SendLittleFSFile(req, index)) return ESP_OK;
        }
        return HttpServer::SendText(req, "404 Not Found", "404: Not Found");
    }

    // ---------------- API ----------------

    void WebSocketAPI::setup(bool failsafeMode) {
        failsafe = failsafeMode;
        if (server != nullptr) return; // keeps running over network down/up, it listens on all interfaces
        if (!HttpServer::Start()) return;

        server = HttpServer::Handle();
        HttpServer::On("/ws", HTTP_GET, WsHandler, /*websocket*/ true);
        HttpServer::SetFallback(FileHandler); // the frontend, LittleFS /api/ overrides and other LittleFS files
        printf("WebSocketAPI: /ws on port %d\r\n", DALHAL_HTTP_PORT);
    }

    bool WebSocketAPI::Broadcast(const char* msg, size_t len, CmdCbType type) {
        if (server == nullptr || msg == nullptr) return false;
        size_t count = CONFIG_LWIP_MAX_SOCKETS;
        int fds[CONFIG_LWIP_MAX_SOCKETS];
        if (httpd_get_client_list(server, &count, fds) != ESP_OK) return false;
        bool anySent = false;
        for (size_t i = 0; i < count; i++) {
            if (httpd_ws_get_fd_info(server, fds[i]) == HTTPD_WS_CLIENT_WEBSOCKET) {
                anySent |= SendFrame(fds[i], msg, len, FrameType(type));
            }
        }
        return anySent;
    }

    bool WebSocketAPI::Broadcast(std::string& msg, CmdCbType type) {
        return Broadcast(msg.c_str(), msg.length(), type);
    }

    bool WebSocketAPI::Broadcast(const char* msg, CmdCbType type) {
        return msg != nullptr && Broadcast(msg, strlen(msg), type);
    }

    bool WebSocketAPI::Broadcast(const ZeroCopyString& zcStr, CmdCbType type) {
        return Broadcast(zcStr.start, zcStr.Length(), type); // no flash strings on ESP32
    }

    bool WebSocketAPI::BroadcastCb(const ZeroCopyString& zcStr, CmdCbType type) {
        return Broadcast(zcStr, type);
    }

    bool WebSocketAPI::Broadcast(const char* source, const char* msg, CmdCbType type) {
        if (source == nullptr || msg == nullptr) return false;
        std::string combined(source);
        combined += msg;
        return Broadcast(combined.c_str(), combined.length(), type);
    }

}
