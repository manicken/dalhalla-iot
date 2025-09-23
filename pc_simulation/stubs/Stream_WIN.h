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

#include <iostream>
#include <iomanip>   // for std::hex, etc.
#include <string>

#ifndef DEC
#define DEC 10
#endif
#ifndef HEX
#define HEX 16
#endif

class Stream {
    std::ostream& out;

public:
    Stream(std::ostream& stream = std::cout) : out(stream) {}
    bool available() { return false; }
    uint8_t read() { return 0; }
    void print(const std::string& s) { out << s; }
    void print(const char* s)        { out << s; }
    void print(char c)               { out << c; }
    void print(int n, int format = DEC) {
        if (format == HEX)
            out << "0x" << std::hex << n << std::dec;
        else
            out << n;
    }
    void println() { out << std::endl; }
    void println(const std::string& s) { out << s << std::endl; }
    void println(const char* s)        { out << s << std::endl; }
};



