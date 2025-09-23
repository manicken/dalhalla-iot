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

#if defined(ESP8266)
#define DEBUG_UART Serial1
#elif defined(ESP32)
#define DEBUG_UART Serial
#endif
#include <cstring>
#include "Logger.h"

namespace Convert
{
    std::string floatToString(float value);
    bool isInteger(const char* str);

    std::string ByteArrayToString(uint8_t* byteArray, size_t arraySize);
    bool HexToBytes(const char* hexString, uint8_t* byteArray, size_t arraySize);

    bool contains(int *array, size_t arraySize, uint8_t value);

    uint64_t reverseMACaddress(uint64_t addr);

    /** with padding */
    String toHex(const char *data, size_t len);
    String convertISO88591toUTF8(const String &input);
    bool IsValidHexString(const char* str);

    std::string toHex(uint8_t value);
    std::string toHex(uint16_t value);
    std::string toHex(uint32_t value);
    std::string toHex(uint64_t value);
    std::string toBin(uint8_t value);


} // namespace Convert