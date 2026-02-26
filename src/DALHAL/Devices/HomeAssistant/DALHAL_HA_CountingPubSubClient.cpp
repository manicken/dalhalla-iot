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

#include "DALHAL_HA_CountingPubSubClient.h"

#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
#include <iostream>
#endif

namespace DALHAL
{
    CountingPubSubClient::CountingPubSubClient() { count=0; }

    size_t CountingPubSubClient::write(uint8_t b) {
        count++;
#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
        std::cout << (char)b;
#elif defined(ESP8266) || defined(ESP32)
        //Serial.write(b);
#endif
        return 1;
    }

    size_t CountingPubSubClient::write(const uint8_t* buffer, size_t size) {
        count += size;
#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
        std::cout.write((const char*)buffer, size);
#elif defined(ESP8266) || defined(ESP32)
        //Serial.write(buffer, size);
#endif
        return size;
    }
}