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

#include "Ticker.h"

void Ticker::onTimer(void* self) {
    Ticker* t = static_cast<Ticker*>(self);
    if (t->cb == nullptr) return;
    if (t->cbNoArg) reinterpret_cast<callback_t>(t->cb)();
    else t->cb(t->cbArg);
}

void Ticker::start(uint32_t ms, bool repeat, cb_arg_t callback, uintptr_t arg, bool noArg) {
    detach();
    cb = callback;
    cbArg = arg;
    cbNoArg = noArg;

    esp_timer_create_args_t args = {};
    args.callback = &Ticker::onTimer;
    args.arg = this;
    args.dispatch_method = ESP_TIMER_TASK;
    args.name = "Ticker";
    if (esp_timer_create(&args, &timer) != ESP_OK) { timer = nullptr; return; }

    const uint64_t us = (uint64_t)ms * 1000ULL;
    if (repeat) esp_timer_start_periodic(timer, us);
    else esp_timer_start_once(timer, us);
}

void Ticker::detach() {
    if (timer == nullptr) return;
    esp_timer_stop(timer);   // returns an error when not running, that is fine
    esp_timer_delete(timer);
    timer = nullptr;
}

bool Ticker::active() const {
    return timer != nullptr && esp_timer_is_active(timer);
}
