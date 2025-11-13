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

#ifndef TICKER_H
#define TICKER_H

#include <Arduino.h>
#include <functional>
#include <thread>
#include <chrono>

class Ticker {
public:
    template<typename TFunc, typename... TArgs>
    void once_ms(unsigned long delayMs, TFunc func, TArgs... args) {
        std::thread([=]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
            func(args...);
        }).detach(); // Detached thread simulates async behavior
    }

    void detach() {
        // No-op for simulation
    }
};

#endif // TICKER_H
