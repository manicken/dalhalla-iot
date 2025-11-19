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

#include <cmath>
#include <stdint.h>
#include <stdio.h>
#include <chrono>
#include <thread>
#include <string>    // std::string for Windows
#include <cstring>
#include <cstdint>
#include "../PrintStream/Stream.h"
#include <cstdarg> // variadic

#define F(x) x

using String = std::string;

#if defined(_WIN32)
#define strdup _strdup
#endif
#define __FlashStringHelper char 

#define pgm_read_byte_near(addr) (*(const unsigned char*)(addr))


#define HIGH 1
#define LOW  0
#define INPUT 0
#define OUTPUT 1

inline void yield() {
    std::this_thread::yield();
}


// Simulate millis() using std::chrono
inline unsigned long millis() {
    static auto start = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    return (unsigned long)std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
}

// Simulate delay() using std::this_thread::sleep_for
inline void delay(unsigned long ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}
inline void delayMicroseconds(unsigned long us) {
    std::this_thread::sleep_for(std::chrono::microseconds(us));
}


// Stub pinMode / digitalWrite / digitalRead
inline void pinMode(int pin, int mode) {
    printf("pinMode(%d, %d)\n", pin, mode);
}
void digitalWrite(int pin, int val);
inline int digitalRead(int pin) {
    int val = 42;
    printf("digitalRead(%d, %d)\n", pin, val);
    return 0; // always low
}
inline void analogWrite(int pin, float val) {
    printf("analogWrite(%d, %f)\n", pin, val);
}

inline float analogRead(int pin) {
    float val = 42.42f;
    printf("analogRead(%d, %f)\n", pin, val);
    return val;
}

#include <iostream>

// Minimal stub for Serial-like behavior
class SerialStub : public Stream {
public:
    void begin(int) {} // No-op for simulation
    void flush() {}
    bool available() { return false; }
    void end() {}
    size_t write(uint8_t b) override { return 1; }
    size_t write(const uint8_t *buffer, size_t size) {return size;}
    uint8_t read() { return 0; }

    template <typename T>
    void print(const T& val) { std::cout << val; }

    template <typename T>
    void println(const T& val) { std::cout << val << std::endl; }

    void println() { std::cout << std::endl; }

    // printf implementation
    void printf(const char* fmt, ...) {
        constexpr size_t bufSize = 512;
        char buffer[bufSize];

        va_list args;
        va_start(args, fmt);
        int n = vsnprintf(buffer, bufSize, fmt, args);
        va_end(args);

        if (n > 0) {
            std::cout.write(buffer, n);
        }
    }

    void printf_ln(const char* fmt, ...) {
        constexpr size_t bufSize = 512;
        char buffer[bufSize];

        va_list args;
        va_start(args, fmt);
        int n = vsnprintf(buffer, bufSize, fmt, args);
        va_end(args);

        if (n > 0) {
            std::cout.write(buffer, n);
        }
        std::cout << std::endl;
    }
};

extern SerialStub Serial; // Declared here, defined in a .cpp

#define HardwareSerial Stream
