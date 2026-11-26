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

#include "IPAddress.h"
#include "Print.h"
#include <cstring>

IPAddress::IPAddress() : octets{0, 0, 0, 0} {}

IPAddress::IPAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d) : octets{a, b, c, d} {}

IPAddress::IPAddress(uint32_t address) {
    memcpy(octets, &address, sizeof(octets));
}

IPAddress::operator uint32_t() const {
    uint32_t address;
    memcpy(&address, octets, sizeof(address));
    return address;
}

bool IPAddress::operator==(const IPAddress& other) const {
    return memcmp(octets, other.octets, sizeof(octets)) == 0;
}

uint8_t IPAddress::operator[](size_t i) const {
    return octets[i];
}

uint8_t& IPAddress::operator[](size_t i) {
    return octets[i];
}

bool IPAddress::fromString(const char* address) {
    if (address == nullptr) return false;
    uint8_t parsed[4];
    int part = 0;
    uint32_t value = 0;
    bool haveDigit = false;
    for (const char* p = address; ; ++p) {
        char c = *p;
        if (c >= '0' && c <= '9') {
            value = value * 10 + (uint32_t)(c - '0');
            if (value > 255) return false;
            haveDigit = true;
        } else if (c == '.' || c == '\0') {
            if (!haveDigit || part > 3) return false;
            parsed[part++] = (uint8_t)value;
            value = 0;
            haveDigit = false;
            if (c == '\0') break;
        } else {
            return false;
        }
    }
    if (part != 4) return false;
    memcpy(octets, parsed, sizeof(octets));
    return true;
}

std::string IPAddress::toString() const {
    return std::to_string(octets[0]) + "." + std::to_string(octets[1]) + "." +
           std::to_string(octets[2]) + "." + std::to_string(octets[3]);
}

size_t IPAddress::printTo(Print& p) const {
    return p.print(toString().c_str());
}
