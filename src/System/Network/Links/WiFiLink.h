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

#if defined(ESP32) || defined(ESP8266)

#include <Arduino.h>
#include <IPAddress.h>

// normal mode: credentials config portal AP, used when STA cannot connect (WiFiConnectionManager)
#ifndef DALHAL_WIFI_PORTAL_AP_SSID
#define DALHAL_WIFI_PORTAL_AP_SSID "DALHAL-Failsafe"
#endif
#ifndef DALHAL_WIFI_PORTAL_AP_PASS
#define DALHAL_WIFI_PORTAL_AP_PASS ""
#endif
#ifndef DALHAL_WIFI_STA_TIMEOUT_MS
#define DALHAL_WIFI_STA_TIMEOUT_MS 8000
#endif
#ifndef DALHAL_WIFI_PORTAL_AP_TIMEOUT_MS
#define DALHAL_WIFI_PORTAL_AP_TIMEOUT_MS 60000
#endif
#ifndef DALHAL_WIFI_RECONNECT_INTERVAL_MS
#define DALHAL_WIFI_RECONNECT_INTERVAL_MS 30000
#endif

// failsafe mode (after crash): AP is always started as a way in, in addition to STA
#ifndef DALHAL_FAILSAFE_AP_SSID
#define DALHAL_FAILSAFE_AP_SSID "Failsafe-Device"
#endif
#ifndef DALHAL_FAILSAFE_AP_PASS
#define DALHAL_FAILSAFE_AP_PASS "12345678"
#endif

namespace Network {
namespace WiFiLink {

    /** settings that must be applied first in setup(), before the crash/failsafe check */
    void PreInit();

    /** normal mode, starts STA with stored credentials and falls back to the config portal AP */
    void Begin();
    void Loop();

    /** failsafe mode, minimal start without cfg dependencies: AP + STA with stored credentials */
    void BeginFailsafe(uint32_t staConnectTimeoutMs);
    void LoopFailsafe();

    bool IsUp();
    IPAddress LocalIP();
    bool HostByName(const char* host, IPAddress& out);

}
}

#endif
