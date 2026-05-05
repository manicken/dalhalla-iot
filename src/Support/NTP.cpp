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

#include "NTP.h"

namespace NTP {

    time_t now;

    void NTPConnect() {
#if defined(DEBUG_UART)
        DEBUG_UART.print(F("Setting time using SNTP  "));
#endif
#if defined(ESP32) || defined(ESP8266)
        configTime(NTP_TIME_ZONE * 3600, 0, "pool.ntp.org", "time.nist.gov");
#endif

        const unsigned long ntpTimeout = 30000; // 30 seconds max wait
        unsigned long start = millis();
        time_t now = time(nullptr);
        time_t nowish = 1609459200; // e.g., Jan 1 2021, safe threshold

        while (now < nowish) {
            if (millis() - start > ntpTimeout) {
#if defined(DEBUG_UART)
                DEBUG_UART.println(F("\nNTP timeout, continuing without valid time"));
#endif
                break;
            }
            delay(500);
#if defined(DEBUG_UART)
            DEBUG_UART.print('.');
#endif
            now = time(nullptr);
        }
#if defined(DEBUG_UART)
        if (now >= nowish) {
            DEBUG_UART.println(F("[OK]"));
            struct tm* timeinfo = localtime(&now);
            DEBUG_UART.print(F("Current time: "));
            DEBUG_UART.println(asctime(timeinfo));
        } else {
            DEBUG_UART.println(F("[NO VALID TIME]"));
        }
#endif
    }

}