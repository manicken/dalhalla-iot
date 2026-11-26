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

#include <string>

/**
  Arduino compatible String on top of std::string, so existing std::string usage keeps working
  and the Arduino number constructors (String(123), String(size)) are available.
  Only single argument number constructors are provided, as (value, base) would be ambiguous
  with std::string(count, char).
*/
class String : public std::string {
public:
    using std::string::string;
    String() = default;
    String(const std::string& s) : std::string(s) {}
    String(std::string&& s) : std::string(std::move(s)) {}

    explicit String(char c) : std::string(1, c) {}
    String(unsigned char v) : std::string(std::to_string(v)) {}
    String(int v) : std::string(std::to_string(v)) {}
    String(unsigned int v) : std::string(std::to_string(v)) {}
    String(long v) : std::string(std::to_string(v)) {}
    String(unsigned long v) : std::string(std::to_string(v)) {}
    String(long long v) : std::string(std::to_string(v)) {}
    String(unsigned long long v) : std::string(std::to_string(v)) {}
    String(float v) : std::string(std::to_string(v)) {}
    String(double v) : std::string(std::to_string(v)) {}
};

// flash is memory mapped (or there is no flash, PC), so flash strings are plain const char*
#define F(x) x
#define __FlashStringHelper char
// tells DALHAL code to skip its const __FlashStringHelper* overloads, they would collide with the const char* ones
#define DALHAL_FLASH_STRING_IS_CHAR