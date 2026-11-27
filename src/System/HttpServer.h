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
  The one HTTP server of the Arduino backend (ESPAsyncWebServer on port 80), the same idea as
  ports/esp-idf/HttpServer: it only owns the server, the modules add their own routes:
    - WebSocketAPI:  /ws and the file fallback (onNotFound: frontend and LittleFS files)
    - FS_Http:       /fs/... (the Files tab of the frontend)
    - System:        /reset, /formatLittleFs
  The owner (main.cpp / failsafe) calls begin() when the network is up.
*/

#pragma once

#include <System/Platform/Platform.h>

#define DALHAL_HTTP_PORT 80

#if defined(DALHAL_BACKEND_ARDUINO)

#include <ESPAsyncWebServer.h>

namespace HttpServer {
    /** the server, created on first use */
    AsyncWebServer& Get();
}

#endif
