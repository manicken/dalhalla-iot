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

#pragma once
#ifdef _WIN32
#ifdef INPUT
#undef INPUT
#endif
#include <httplib.h>  // Add to your project
#endif
#include <functional>
#include <string>

#define DALHAL_REST_API_PORT 82

using DALHAL_REST_CB = std::function<std::string(const std::string&)>;

namespace DALHAL {
    class REST {
    private:
#ifdef _WIN32
        static httplib::Server server;
#endif
        static DALHAL_REST_CB callback;
    public:
        static void setup(DALHAL_REST_CB cb);
    };
}