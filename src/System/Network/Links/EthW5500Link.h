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
  W5500 over SPI in raw MAC mode (ESP-IDF esp_eth driver) under the ESP32 lwIP stack,
  the WIZnet on-chip TCP/IP stack is not used. Requires arduino-esp32 3.x.

  All settings are build flags only (no LittleFS/cfg.json dependency), so the link
  also works in failsafe mode after a crash.

  Required:
    -D DALHAL_NET_LINK_W5500
    -D DALHAL_ETH_W5500_CS=<pin> -D DALHAL_ETH_W5500_IRQ=<pin> -D DALHAL_ETH_W5500_RST=<pin or -1>
    -D DALHAL_ETH_W5500_SCK=<pin> -D DALHAL_ETH_W5500_MISO=<pin> -D DALHAL_ETH_W5500_MOSI=<pin>
  Optional static fallback, used when DHCP gives no address within DALHAL_ETH_DHCP_TIMEOUT_MS:
    -D DALHAL_ETH_FALLBACK_IP=\"192.168.1.250\" -D DALHAL_ETH_FALLBACK_GATEWAY=\"192.168.1.1\"
    -D DALHAL_ETH_FALLBACK_SUBNET=\"255.255.255.0\" -D DALHAL_ETH_FALLBACK_DNS=\"192.168.1.1\"
*/

#if defined(ESP32) && defined(DALHAL_NET_LINK_W5500)

#include <Arduino.h>
#include <IPAddress.h>

#if !defined(DALHAL_ETH_W5500_CS) || !defined(DALHAL_ETH_W5500_IRQ) || !defined(DALHAL_ETH_W5500_RST) || \
    !defined(DALHAL_ETH_W5500_SCK) || !defined(DALHAL_ETH_W5500_MISO) || !defined(DALHAL_ETH_W5500_MOSI)
#error "DALHAL_NET_LINK_W5500 requires DALHAL_ETH_W5500_CS/IRQ/RST/SCK/MISO/MOSI build flags"
#endif

#ifndef DALHAL_ETH_W5500_PHY_ADDR
#define DALHAL_ETH_W5500_PHY_ADDR 1
#endif
#ifndef DALHAL_ETH_W5500_SPI_MHZ
#define DALHAL_ETH_W5500_SPI_MHZ 20
#endif
#ifndef DALHAL_ETH_DHCP_TIMEOUT_MS
#define DALHAL_ETH_DHCP_TIMEOUT_MS 10000
#endif

namespace Network {
namespace EthW5500Link {

    /** normal mode, starts the W5500 and waits for DHCP (or fallback IP) until DALHAL_ETH_DHCP_TIMEOUT_MS */
    void Begin();
    void Loop();

    /** failsafe mode, same as normal mode as Ethernet has no extra failsafe access path */
    void BeginFailsafe();
    void LoopFailsafe();

    bool IsUp();
    IPAddress LocalIP();

}
}

#endif
