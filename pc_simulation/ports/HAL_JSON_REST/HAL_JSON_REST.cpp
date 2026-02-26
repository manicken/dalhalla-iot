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

#ifdef _WIN32
#ifdef INPUT
#undef INPUT
#endif
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#endif

#include "DALHAL_REST.h"

//using namespace httplib;
#ifdef _WIN32
static httplib::Server server;
#endif

namespace DALHAL {

    // Define the static server
    DALHAL_REST_CB REST::callback;  // define the static member here
#ifdef _WIN32
    httplib::Server REST::server;
#endif
    void REST::setup(DALHAL_REST_CB cb) {
        
        REST::callback = cb;
#ifdef _WIN32
        // Example route
        server.set_error_handler([](const httplib::Request& req, httplib::Response& res) {
            //if (res.status != 404) return; // only intercept 404 errors
            std::cout << "req.path:" << req.path << "\n";
            res.set_content(REST::callback(req.path), "application/json");
        });
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            std::cerr << "WSAStartup failed: " << result << std::endl;
            return; // exit thread early if failed
        }
        std::cout << "Winsock initialized.\n";

        int port = server.bind_to_port("0.0.0.0", DALHAL_REST_API_PORT);
        if (port > 0) {
            std::cout << "*** REST API server started on port " << DALHAL_REST_API_PORT << "\n";
            std::cout << "*** can be accessed by http://localhost:82 \n";
            std::thread serverThread([]() {
                server.listen_after_bind();
                WSACleanup();
                std::cout << "Winsock cleaned up.\n";
            });
            serverThread.detach();
        } else {
            WSACleanup();
            std::cerr << "Failed to bind REST API server.\n";
        }
#endif
    }
}
