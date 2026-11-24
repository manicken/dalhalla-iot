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

#include "EthW5500Link.h"

#if defined(ESP32) && defined(DALHAL_NET_LINK_W5500)

#include <ETH.h>
#include <SPI.h>

namespace Network {
namespace EthW5500Link {

    static bool started = false;
    static bool fallbackApplied = false;
    static uint32_t linkUpSinceMs = 0;
    static bool wasLinkUp = false;

    static void Start() {
        if (started) { return; }
        SPI.begin(DALHAL_ETH_W5500_SCK, DALHAL_ETH_W5500_MISO, DALHAL_ETH_W5500_MOSI);
        started = ETH.begin(ETH_PHY_W5500, DALHAL_ETH_W5500_PHY_ADDR,
                            DALHAL_ETH_W5500_CS, DALHAL_ETH_W5500_IRQ, DALHAL_ETH_W5500_RST,
                            SPI, DALHAL_ETH_W5500_SPI_MHZ);
        if (started) {
            Serial.println(F("[ETH] W5500 started, waiting for link/DHCP"));
        } else {
            Serial.println(F("[ETH] ERROR: W5500 could not be started (check wiring/pins)"));
        }
    }

    /** DHCP gave no address within the timeout after link up, use the static fallback (once per boot) */
    static void ApplyFallbackIfNeeded() {
#if defined(DALHAL_ETH_FALLBACK_IP)
        bool linkUp = ETH.linkUp();
        if (linkUp && !wasLinkUp) { linkUpSinceMs = millis(); }
        wasLinkUp = linkUp;

        if (fallbackApplied || !linkUp || ETH.hasIP()) { return; }
        if ((millis() - linkUpSinceMs) < DALHAL_ETH_DHCP_TIMEOUT_MS) { return; }

        IPAddress ip, gateway, subnet, dns;
        ip.fromString(DALHAL_ETH_FALLBACK_IP);
#if defined(DALHAL_ETH_FALLBACK_GATEWAY)
        gateway.fromString(DALHAL_ETH_FALLBACK_GATEWAY);
#endif
#if defined(DALHAL_ETH_FALLBACK_SUBNET)
        subnet.fromString(DALHAL_ETH_FALLBACK_SUBNET);
#else
        subnet = IPAddress(255, 255, 255, 0);
#endif
#if defined(DALHAL_ETH_FALLBACK_DNS)
        dns.fromString(DALHAL_ETH_FALLBACK_DNS);
#endif
        fallbackApplied = true;
        if (ETH.config(ip, gateway, subnet, dns)) {
            Serial.print(F("[ETH] no DHCP answer, using fallback IP: ")); Serial.println(ip);
        } else {
            Serial.println(F("[ETH] ERROR: could not apply fallback IP"));
        }
#endif
    }

    void Begin() {
        Start();
        if (!started) { return; }
        // link negotiation + DHCP, the fallback timer starts at link up
        uint32_t t = millis();
        while ((millis() - t) < (DALHAL_ETH_DHCP_TIMEOUT_MS + 3000) && ETH.hasIP() == false) {
            ApplyFallbackIfNeeded();
            delay(10);
        }
        if (ETH.hasIP()) {
            Serial.print(F("[ETH] connected, IP: ")); Serial.println(ETH.localIP());
        } else {
            Serial.println(F("[ETH] no IP yet (cable/DHCP), will keep trying in background"));
        }
    }

    void Loop() {
        if (!started) { return; }
        ApplyFallbackIfNeeded();
    }

    void BeginFailsafe() {
        Begin();
    }

    void LoopFailsafe() {
        Loop();
    }

    bool IsUp() {
        return started && ETH.linkUp() && ETH.hasIP();
    }

    IPAddress LocalIP() {
        return ETH.localIP();
    }

}
}

#endif
