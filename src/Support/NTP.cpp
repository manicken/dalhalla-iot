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
#include <System/Platform/Platform.h>

#if defined(DALHAL_BACKEND_ESPIDF)
#include <cstdio>
#include <cstdlib>
#include "esp_sntp.h"
#endif

namespace NTP {

    time_t now;

#if defined(DALHAL_BACKEND_ESPIDF)
    /** what arduino-esp32 configTime(gmtOffset, 0, server1, server2) does, on plain ESP-IDF */
    static void ConfigTime(long gmtOffsetSec, const char* server1, const char* server2) {
        // POSIX TZ has the sign inverted: UTC+1 is "UTC-01:00"
        char tz[32];
        long offsetMin = gmtOffsetSec / 60;
        char sign = offsetMin >= 0 ? '-' : '+';
        if (offsetMin < 0) offsetMin = -offsetMin;
        snprintf(tz, sizeof(tz), "UTC%c%02ld:%02ld", sign, offsetMin / 60, offsetMin % 60);
        setenv("TZ", tz, 1);
        tzset();

        if (esp_sntp_enabled()) esp_sntp_stop();
        esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
        esp_sntp_setservername(0, server1);
        esp_sntp_setservername(1, server2);
        esp_sntp_init();
    }
#endif

    void NTPConnect() {
#if defined(DEBUG_UART)
        DEBUG_UART.print(F("Setting time using SNTP  "));
#endif
#if defined(DALHAL_BACKEND_ESPIDF)
        ConfigTime(NTP_TIME_ZONE * 3600, "pool.ntp.org", "time.nist.gov");
#elif defined(ESP32) || defined(ESP8266)
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