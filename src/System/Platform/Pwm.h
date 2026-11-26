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

#include "Platform.h"

namespace Platform {
namespace Pwm {

    /** a PWM output, channel is only used on platforms with explicit channels (ESP32 LEDC) */
    struct Output {
        uint8_t pin;
        uint8_t channel;
    };

    /** configures the output, on ESP8266 frequency and resolution are global for all pins */
    bool Attach(const Output& out, uint32_t freqHz, uint8_t resolutionBits);
    /** duty in the range 0 .. (2^resolutionBits - 1) */
    void Write(const Output& out, uint32_t duty);
    /** stops the PWM and releases the pin */
    void Detach(const Output& out);

}
}
