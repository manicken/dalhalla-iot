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

#include <cstdint>
#include <cstddef>
#include <string>
#include "Printable.h"

/**
  IPv4 address, the part of Arduino's IPAddress that DALHAL uses.
  The uint32_t form is the address in network byte order as it lies in memory (as lwIP's ip4_addr_t.addr),
  the same as the Arduino core: IPAddress(192,168,1,2) == (uint32_t)0x0201A8C0 on little endian.
*/
class IPAddress : public Printable {
public:
    IPAddress();
    IPAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d);
    IPAddress(uint32_t address);

    operator uint32_t() const;
    bool operator==(const IPAddress& other) const;
    bool operator!=(const IPAddress& other) const { return !(*this == other); }

    uint8_t operator[](size_t i) const;
    uint8_t& operator[](size_t i);

    /** parses "a.b.c.d", returns false (and keeps the old value) when it is not a valid address */
    bool fromString(const char* address);

    std::string toString() const;
    size_t printTo(Print& p) const override;

private:
    uint8_t octets[4];
};
