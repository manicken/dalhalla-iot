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

#include "Arduino.h"

#include <cstdio>
#include "driver/gpio.h"

// ---------------- GPIO ----------------

void pinMode(uint8_t pin, uint8_t mode) {
    gpio_config_t cfg = {};
    cfg.pin_bit_mask = 1ULL << pin;
    cfg.intr_type = GPIO_INTR_DISABLE;
    cfg.pull_up_en = GPIO_PULLUP_DISABLE;
    cfg.pull_down_en = GPIO_PULLDOWN_DISABLE;
    switch (mode) {
        case OUTPUT:         cfg.mode = GPIO_MODE_INPUT_OUTPUT; break; // INPUT_OUTPUT so digitalRead returns the output level, as on the Arduino core
        case INPUT_PULLUP:   cfg.mode = GPIO_MODE_INPUT; cfg.pull_up_en = GPIO_PULLUP_ENABLE; break;
        case INPUT_PULLDOWN: cfg.mode = GPIO_MODE_INPUT; cfg.pull_down_en = GPIO_PULLDOWN_ENABLE; break;
        case INPUT:
        default:             cfg.mode = GPIO_MODE_INPUT; break;
    }
    gpio_config(&cfg);
}

void digitalWrite(uint8_t pin, uint8_t val) {
    gpio_set_level((gpio_num_t)pin, val ? 1 : 0);
}

int digitalRead(uint8_t pin) {
    return gpio_get_level((gpio_num_t)pin);
}

int analogRead(uint8_t /*pin*/) {
    return 0;
}

// ---------------- Serial ----------------

ConsoleSerial Serial;
ConsoleSerial Serial1;

void ConsoleSerial::flush() {
    fflush(stdout);
}

int ConsoleSerial::peek() {
    if (peeked < 0) {
        int c = fgetc(stdin); // the console VFS is non blocking, EOF when nothing is received
        if (c == EOF) { clearerr(stdin); return -1; }
        peeked = c;
    }
    return peeked;
}

int ConsoleSerial::available() {
    return peek() < 0 ? 0 : 1;
}

int ConsoleSerial::read() {
    int c = peek();
    peeked = -1;
    return c;
}

size_t ConsoleSerial::write(uint8_t b) {
    return fputc(b, stdout) == EOF ? 0 : 1;
}

size_t ConsoleSerial::write(const uint8_t* buffer, size_t size) {
    return fwrite(buffer, 1, size, stdout);
}
