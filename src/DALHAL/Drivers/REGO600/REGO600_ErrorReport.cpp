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

#include "REGO600_ErrorReport.h"

#include "stdint.h"
#include <DALHAL/API/DALHAL_WebSocketAPI.h>
#include <DALHAL/Support/DALHAL_Logger.h>

#define DRIVERS_REGO600_ERROR_BASE_STR "REGO600 error - "

namespace Drivers {
    namespace REGO600 {

        // can now be true as we have fixed major comm errors, and if there is any future we want to know that directly
        // however it can still be deactivated by DALHAL REGO600 device layer if it's very annoying
        bool ErrorReport::emitErrorsToWebSocket = true;

        void ErrorReport::DebugMessage(const char* msg) {
            if (emitErrorsToWebSocket) {
                DALHAL::WebSocketAPI::Broadcast(DRIVERS_REGO600_ERROR_BASE_STR, msg);
            }
            GlobalLogger.Error(F(DRIVERS_REGO600_ERROR_BASE_STR), msg);
        }

    }
}