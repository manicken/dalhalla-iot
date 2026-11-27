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

/**
  The one HTTP server of the ESP-IDF backend (esp_http_server on port 80).
  It only owns the server, the modules add their own routes:
    - WebSocketAPI:  /ws and the built in frontend (as the fallback)
    - FS_Http:       /fs/...
    - HttpServer:    /reset
  Routes are matched in the order they are added, the fallback (GET of anything else) is always last.
*/

#pragma once

#include <esp_http_server.h>

#define DALHAL_HTTP_PORT 80

namespace HttpServer {

    using Handler = esp_err_t (*)(httpd_req_t* req);

    /** starts the server, more calls do nothing; returns false when it could not start */
    bool Start();
    /** nullptr until Start() succeeded */
    httpd_handle_t Handle();

    /** adds a route, matched before the fallback, the uri may end with a * wildcard */
    bool On(const char* uri, httpd_method_t method, Handler handler, bool websocket = false);
    /** handles a GET that no route matched, without one the answer is 404 */
    void SetFallback(Handler handler);

    /** value of a query parameter, url decoded; false when it is not there */
    bool GetQueryParam(httpd_req_t* req, const char* key, char* out, size_t outSize);
    /** sends a plain text answer with the given status, e.g. "404 Not Found" */
    esp_err_t SendText(httpd_req_t* req, const char* status, const char* text);
}
