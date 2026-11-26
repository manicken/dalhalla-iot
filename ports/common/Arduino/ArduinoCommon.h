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

/**
  The platform independent part of the Arduino core subset (contract layer B).
  Every non Arduino port provides its own Arduino.h that includes this file and then adds
  the platform specific part: millis/micros/delay/yield, the GPIO functions and Serial.
*/

#pragma once

#include <cmath>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cstdarg>
#include <cstdio>
#include <string>

#include <pgmspace.h> // PROGMEM, PGM_P, *_P functions
#include <WString.h>  // String, F(), __FlashStringHelper
#include <Print.h>

#define HIGH 1
#define LOW  0
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLDOWN 0x09
#define INPUT_PULLUP 0x05

typedef uint8_t byte;

/** Arduino constrain, as a template instead of a macro so arguments are evaluated once */
template<typename T, typename L, typename H>
inline T constrain(T value, L low, H high) {
    return value < (T)low ? (T)low : (value > (T)high ? (T)high : value);
}
