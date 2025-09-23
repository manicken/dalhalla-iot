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

#include <Arduino.h>

struct TempAndHumidity {
  float temperature;
  float humidity;
};

class DHTesp {
public:
    typedef enum {
        AUTO_DETECT,
        DHT11,
        DHT22,
        AM2302,  // Packaged DHT22
        RHT03    // Equivalent to DHT22
    }
    DHT_MODEL_t;

    typedef enum {
        ERROR_NONE = 0,
        ERROR_TIMEOUT,
        ERROR_CHECKSUM
    }
    DHT_ERROR_t;

    DHTesp();

    void setup(uint8_t pin, DHT_MODEL_t model);
    TempAndHumidity getTempAndHumidity();
};