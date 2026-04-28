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

#pragma once

#include <cstdint> // C Standard Integer Types

namespace DALHAL {

#if defined(ESP8266)
    enum class gpio_num_t : int8_t {
        GPIO_NUM_NC = -1,
        GPIO0 = 0,
        GPIO1 = 1,
        GPIO2 = 2,
        GPIO3 = 3,
        GPIO4 = 4,
        GPIO5 = 5,
        GPIO12 = 12,
        GPIO13 = 13,
        GPIO14 = 14,
        GPIO15 = 15,
        GPIO16 = 16
    };
#endif
}