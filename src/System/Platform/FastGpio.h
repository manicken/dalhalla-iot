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

/*
  GPIO access with edge interrupts and ISR safe writes, used by devices that must react
  directly in the ISR (e.g. Actuator/LatchingRelay end stops that kill the motor drive).

  Backends: ESP32 (ESP-IDF gpio driver + gpio_ll), PC (no-op).
  ESP8266 is not implemented (no-op), as before the devices using this did nothing there.
*/

#include "Platform.h"

#if defined(DALHAL_PLATFORM_ESP32) || defined(DALHAL_PLATFORM_ESP8266)
#include <Arduino.h> // IRAM_ATTR
#define PLATFORM_ISR_ATTR IRAM_ATTR
#else
#define PLATFORM_ISR_ATTR
#endif

namespace Platform {
namespace FastGpio {

#if defined(DALHAL_PLATFORM_ESP32)
    constexpr bool Available = true;
#else
    /** false means the functions are no-ops, devices must then refuse to drive outputs */
    constexpr bool Available = false;
#endif

    using Pin = int16_t;
    /** not connected */
    constexpr Pin NC = -1;

    enum class Edge : uint8_t { Rising, Falling };
    using IsrHandler = void (*)(void* arg);

    /** output, no pulls, interrupt disabled */
    void ConfigureOutput(Pin pin);
    /** releases the pin: input, no pulls, interrupt disabled */
    void Release(Pin pin);

    void Write(Pin pin, bool level);
    bool Read(Pin pin);

    /** input without pulls and an edge interrupt calling handler(arg), the interrupt is enabled */
    bool AttachEdgeIsr(Pin pin, Edge edge, IsrHandler handler, void* arg);
    void DetachIsr(Pin pin);
    void IsrEnable(Pin pin);
    /** ISR safe */
    void PLATFORM_ISR_ATTR IsrDisable(Pin pin);

    /** ISR safe minimal write (single register store on ESP32) */
    void PLATFORM_ISR_ATTR IsrWrite(Pin pin, bool level);

}
}
