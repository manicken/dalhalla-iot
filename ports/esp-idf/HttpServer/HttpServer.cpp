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

#include "HttpServer.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <Arduino.h> // delay
#include <System/Platform/Platform.h>

namespace HttpServer {

    static httpd_handle_t server = nullptr;
    static Handler fallback = nullptr;

    static esp_err_t FallbackHandler(httpd_req_t* req) {
        if (fallback != nullptr) return fallback(req);
        return SendText(req, "404 Not Found", "404: Not Found");
    }

    static void RegisterFallback() {
        httpd_uri_t any = {};
        any.uri = "/*";
        any.method = HTTP_GET;
        any.handler = FallbackHandler;
        httpd_register_uri_handler(server, &any);
    }

    static esp_err_t ResetHandler(httpd_req_t* req) {
        SendText(req, "200 OK", "Restarting...");
        delay(200);
        Platform::Restart();
        return ESP_OK;
    }

    bool Start() {
        if (server != nullptr) return true; // keeps running over network down/up, it listens on all interfaces

        httpd_config_t config = HTTPD_DEFAULT_CONFIG();
        config.server_port = DALHAL_HTTP_PORT;
        config.uri_match_fn = httpd_uri_match_wildcard;
        config.lru_purge_enable = true;  // a browser keeps idle connections open, drop the oldest when full
        config.stack_size = 6144;
        config.max_uri_handlers = 16;
        // a browser keeps several keep-alive connections open next to the WebSocket(s);
        // lwIP needs free sockets above this for httpd itself, OTA, MQTT ... (CONFIG_LWIP_MAX_SOCKETS=20)
        config.max_open_sockets = 12;

        if (httpd_start(&server, &config) != ESP_OK) {
            printf("HttpServer: httpd_start failed\r\n");
            server = nullptr;
            return false;
        }
        RegisterFallback();
        On("/reset", HTTP_GET, ResetHandler);
        printf("HttpServer: listening on port %d\r\n", DALHAL_HTTP_PORT);
        return true;
    }

    httpd_handle_t Handle() {
        return server;
    }

    bool On(const char* uri, httpd_method_t method, Handler handler, bool websocket) {
        if (server == nullptr) return false;
        // esp_http_server matches in registration order, so the /* fallback is moved to the end again
        httpd_unregister_uri_handler(server, "/*", HTTP_GET);
        httpd_uri_t route = {};
        route.uri = uri;
        route.method = method;
        route.handler = handler;
        route.is_websocket = websocket;
        esp_err_t err = httpd_register_uri_handler(server, &route);
        RegisterFallback();
        if (err != ESP_OK && err != ESP_ERR_HTTPD_HANDLER_EXISTS) {
            printf("HttpServer: could not add %s (%d)\r\n", uri, (int)err);
            return false;
        }
        return true;
    }

    void SetFallback(Handler handler) {
        fallback = handler;
    }

    static int HexValue(char c) {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    }

    bool GetQueryParam(httpd_req_t* req, const char* key, char* out, size_t outSize) {
        size_t len = httpd_req_get_url_query_len(req);
        if (len == 0 || outSize == 0) return false;
        char* query = static_cast<char*>(malloc(len + 1));
        if (query == nullptr) return false;
        bool found = httpd_req_get_url_query_str(req, query, len + 1) == ESP_OK &&
                     httpd_query_key_value(query, key, out, outSize) == ESP_OK;
        free(query);
        if (!found) return false;

        // url decode in place, %XX and +
        char* w = out;
        for (const char* r = out; *r; r++) {
            if (*r == '%' && HexValue(r[1]) >= 0 && HexValue(r[2]) >= 0) {
                *w++ = (char)(HexValue(r[1]) * 16 + HexValue(r[2]));
                r += 2;
            } else if (*r == '+') {
                *w++ = ' ';
            } else {
                *w++ = *r;
            }
        }
        *w = '\0';
        return true;
    }

    esp_err_t SendText(httpd_req_t* req, const char* status, const char* text) {
        httpd_resp_set_status(req, status);
        httpd_resp_set_type(req, "text/plain");
        return httpd_resp_sendstr(req, text);
    }
}
