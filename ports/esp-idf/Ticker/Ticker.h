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
  Arduino Ticker on esp_timer (the same as arduino-esp32 does).
  Callbacks run in the esp_timer task, keep them short.
*/

#pragma once

#include <cstdint>
#include "esp_timer.h"

class Ticker {
public:
    Ticker() = default;
    Ticker(const Ticker&) = delete;
    Ticker& operator=(const Ticker&) = delete;
    ~Ticker() { detach(); }

    using callback_t = void (*)();

    void once_ms(uint32_t ms, callback_t callback) { start(ms, false, reinterpret_cast<cb_arg_t>(callback), 0, true); }
    void attach_ms(uint32_t ms, callback_t callback) { start(ms, true, reinterpret_cast<cb_arg_t>(callback), 0, true); }

    /** callback with one pointer sized argument, as arduino-esp32 */
    template<typename TArg>
    void once_ms(uint32_t ms, void (*callback)(TArg), TArg arg) {
        static_assert(sizeof(TArg) <= sizeof(uintptr_t), "Ticker argument must fit in a pointer");
        start(ms, false, reinterpret_cast<cb_arg_t>(callback), (uintptr_t)arg, false);
    }
    template<typename TArg>
    void attach_ms(uint32_t ms, void (*callback)(TArg), TArg arg) {
        static_assert(sizeof(TArg) <= sizeof(uintptr_t), "Ticker argument must fit in a pointer");
        start(ms, true, reinterpret_cast<cb_arg_t>(callback), (uintptr_t)arg, false);
    }

    void detach();
    bool active() const;

private:
    using cb_arg_t = void (*)(uintptr_t);
    void start(uint32_t ms, bool repeat, cb_arg_t callback, uintptr_t arg, bool noArg);
    static void onTimer(void* self);

    esp_timer_handle_t timer = nullptr;
    cb_arg_t cb = nullptr;
    uintptr_t cbArg = 0;
    bool cbNoArg = false;
};
