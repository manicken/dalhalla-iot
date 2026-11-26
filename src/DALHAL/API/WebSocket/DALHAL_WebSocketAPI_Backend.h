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
  Includes the WebSocketAPI of the current backend, include this instead of a backend specific header.
  All of them provide DALHAL::WebSocketAPI with the Broadcast functions.
*/

#pragma once

#include <System/Platform/Platform.h>

#if defined(DALHAL_BACKEND_ARDUINO)
#include <DALHAL/API/WebSocket/DALHAL_WebSocketAPI.h>  // ESPAsyncWebServer
#elif defined(DALHAL_BACKEND_PC)
#include <DALHAL_WebSocketAPI_PC.h>                    // ports/pc, winsock
#elif defined(DALHAL_BACKEND_ESPIDF)
#include <DALHAL_WebSocketAPI_ESPIDF.h>                // ports/esp-idf
#else
#error "no WebSocketAPI for this backend"
#endif
