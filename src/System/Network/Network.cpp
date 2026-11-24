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

#include "Network.h"

#if defined(ESP32) || defined(ESP8266)
// no link selected by build flags means WiFi only (the original behavior)
#if !defined(DALHAL_NET_LINK_WIFI) && !defined(DALHAL_NET_LINK_W5500)
#define DALHAL_NET_LINK_WIFI
#endif
#include "Links/WiFiLink.h" // always included, the WiFi failsafe AP is used as a backup way in on all ESP targets
#include "Links/EthW5500Link.h"
#elif defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
#include <WiFi.h>
#endif

#ifndef DALHAL_FAILSAFE_STA_CONNECT_TIMEOUT_MS
#define DALHAL_FAILSAFE_STA_CONNECT_TIMEOUT_MS 60000
#endif

namespace Network {

#if defined(ESP32) || defined(ESP8266)

    bool IsUp() {
#if defined(DALHAL_NET_LINK_W5500)
        if (EthW5500Link::IsUp()) { return true; }
#endif
#if defined(DALHAL_NET_LINK_WIFI)
        if (WiFiLink::IsUp()) { return true; }
#endif
        return false;
    }

    /** the IP of the preferred link that is up (Ethernet before WiFi) */
    IPAddress LocalIP() {
#if defined(DALHAL_NET_LINK_W5500)
        if (EthW5500Link::IsUp()) { return EthW5500Link::LocalIP(); }
#endif
#if defined(DALHAL_NET_LINK_WIFI)
        if (WiFiLink::IsUp()) { return WiFiLink::LocalIP(); }
#endif
        return IPAddress();
    }

    bool HostByName(const char* host, IPAddress& out) {
        // the DNS resolver is shared by all lwIP interfaces
        return WiFiLink::HostByName(host, out);
    }

    static EventCallback eventCallback = nullptr;
    static bool wasUp = false;

    /** emits Up/Down on every transition, so services are restarted after each reconnect (as before) */
    static void CheckUpDown() {
        bool up = IsUp();
        if (up == wasUp) { return; }
        wasUp = up;
        if (eventCallback != nullptr) {
            eventCallback(up ? Event::Up : Event::Down);
        }
    }

    void SetEventCallback(EventCallback callback) {
        eventCallback = callback;
    }

    void PreInit() {
        WiFiLink::PreInit();
    }

    void Begin() {
#if defined(DALHAL_NET_LINK_W5500)
        EthW5500Link::Begin();
#endif
#if defined(DALHAL_NET_LINK_WIFI)
        WiFiLink::Begin();
#endif
        CheckUpDown();
    }

    void Loop() {
#if defined(DALHAL_NET_LINK_W5500)
        EthW5500Link::Loop();
#endif
#if defined(DALHAL_NET_LINK_WIFI)
        WiFiLink::Loop();
#endif
        CheckUpDown();
    }

    void BeginFailsafe() {
        // Ethernet first as it is the primary way in, the WiFi failsafe AP is always started as backup
#if defined(DALHAL_NET_LINK_W5500)
        EthW5500Link::BeginFailsafe();
#endif
        WiFiLink::BeginFailsafe(DALHAL_FAILSAFE_STA_CONNECT_TIMEOUT_MS);
    }

    void LoopFailsafe() {
#if defined(DALHAL_NET_LINK_W5500)
        EthW5500Link::LoopFailsafe();
#endif
        WiFiLink::LoopFailsafe();
    }

#elif defined(_WIN32) || defined(__linux__) || defined(__APPLE__)

    bool IsUp() {
        return WiFi.status() == WL_CONNECTED;
    }

    IPAddress LocalIP() {
        return WiFi.localIP();
    }

    bool HostByName(const char* host, IPAddress& out) {
        return WiFi.hostByName(host, out) == 1;
    }

#endif

}
