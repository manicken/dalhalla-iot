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
  Link independent network API for network consumers (HomeAssistant, ThingSpeak, REST, OTA ...).
  Consumers must use this instead of WiFi.* so that they also work over Ethernet links
  (W5500/ENC28J60 in raw MAC mode, LAN8720, Teensy 4.1 native Ethernet).

  Link specifics (WiFi credentials portal, failsafe AP ...) are handled inside each link (Links/),
  this API only exposes the link independent lifecycle and the Up/Down events.
*/

#include <Arduino.h>
#include <IPAddress.h>

#if defined(ESP32) || defined(ESP8266) || defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
#include <WiFiClient.h>
#endif

namespace Network {

#if defined(ESP32) || defined(ESP8266) || defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
    /** WiFiClient is a plain lwIP socket on these platforms, so it also works over Ethernet links */
    using TcpClient = WiFiClient;
#else
#error "Network::TcpClient is not defined for this platform"
#endif

    /** true when any network link is up and has an IP address */
    bool IsUp();
    IPAddress LocalIP();
    /** resolves host (or parses an IP literal) into out, returns false on failure */
    bool HostByName(const char* host, IPAddress& out);

#if defined(ESP32) || defined(ESP8266)
    enum class Event {
        /** network went from down to up, network services should be (re)started */
        Up,
        Down
    };
    using EventCallback = void (*)(Event event);
    void SetEventCallback(EventCallback callback);

    /** must be called first in setup(), before the crash/failsafe check */
    void PreInit();

    /** normal mode, starts all links, blocks until up or link connect timeout */
    void Begin();
    /** call from main loop, runs the link state machines and emits Up/Down */
    void Loop();

    /** failsafe mode (after crash), minimal link start without any cfg dependencies */
    void BeginFailsafe();
    /** call from the failsafe loop, keeps the links alive */
    void LoopFailsafe();
#endif

}
