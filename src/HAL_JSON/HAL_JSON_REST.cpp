/*
  Dalhalla IoT — JSON-configured HAL/DAL + Script Engine
  HAL = Hardware Abstraction Layer
  DAL = Device Abstraction Layer

  Provides IoT firmware building blocks for home automation and smart sensors.

  Copyright (C) 2025 Jannik Svensson

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

#include "HAL_JSON_REST.h"

#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"

namespace HAL_JSON {

    AsyncWebServer* REST::asyncWebserver = nullptr;

    void DefaultHandler(AsyncWebServerRequest *request) {
        const char* urlStr = request->url().c_str();

        if (urlStr == nullptr || *urlStr == '\0') {
            request->send(200, "application/json", "{\"error\":\"urlEmpty\"}");
            return;
        }
        if (*(urlStr + 1) == '\0') {
            request->send(200, "application/json", "{\"error\":\"emptyPath\"}");
            return;
        }

        AsyncResponseStream* response = request->beginResponseStream("application/json");

        response->setCode(200);

        // Send headers immediately
        request->send(response);

        CommandExecutor_LOCK_QUEUE();
        CommandExecutor::g_pending.push({
            std::string(request->url().c_str() + 1),
            [response](const std::string& body) {
                
                response->print(body.c_str());
                //response->flush();
            }
        });
        CommandExecutor_UNLOCK_QUEUE();

        /* this causes random crashes
        // Capture request in lambda for later response
        CommandExecutor_LOCK_QUEUE();
        CommandExecutor::g_pending.push({std::string(urlStr + 1), 
            [request](const std::string& response) {

                if (request->client()->connected()) {

                    request->send(200, "application/json", response.c_str());
                }
            }
        });
        CommandExecutor_UNLOCK_QUEUE();
        */
    }

    void REST::setupRest() {
        asyncWebserver = new AsyncWebServer(82);
        asyncWebserver->onNotFound(DefaultHandler);
        asyncWebserver->begin();
    }
}
