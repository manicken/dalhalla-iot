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
  ESP-IDF part of the Arduino core subset (contract layer B), no Arduino core involved:
  time via esp_timer/FreeRTOS, GPIO via the IDF gpio driver, Serial on the console (stdout/stdin).
  The platform independent part is in ports/common/Arduino/ArduinoCommon.h
*/

#pragma once

#include <ArduinoCommon.h>
#include <Stream.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_rom_sys.h"

// ---------------- time ----------------

inline unsigned long millis() { return (unsigned long)(esp_timer_get_time() / 1000ULL); }
inline unsigned long micros() { return (unsigned long)esp_timer_get_time(); }
inline void delay(unsigned long ms) { vTaskDelay(pdMS_TO_TICKS(ms)); }
inline void delayMicroseconds(unsigned int us) { esp_rom_delay_us(us); }
/** like the Arduino core: gives other tasks of the same priority a chance to run */
inline void yield() { vPortYield(); }

// ---------------- GPIO ----------------

void pinMode(uint8_t pin, uint8_t mode);
void digitalWrite(uint8_t pin, uint8_t val);
int digitalRead(uint8_t pin);
/** TODO: ADC oneshot driver, returns 0 until then */
int analogRead(uint8_t pin);

// ---------------- Serial ----------------

/** Serial on the ESP-IDF console (UART0 by default, stdout/stdin through the VFS) */
class ConsoleSerial : public Stream {
public:
    void begin(unsigned long /*baud*/) {} // the console UART is set up by ESP-IDF (CONFIG_ESP_CONSOLE_UART_BAUDRATE)
    void end() {}
    void flush() override;
    int available() override;
    int read() override;
    int peek() override;
    size_t write(uint8_t b) override;
    size_t write(const uint8_t* buffer, size_t size) override;
    using Print::write;
private:
    int peeked = -1;
};

extern ConsoleSerial Serial;
#define HardwareSerial Stream
