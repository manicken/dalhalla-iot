/*
  Dalhalla IoT — JSON-configured HAL/DAL + Script Engine
  HAL = Hardware Abstraction Layer
  DAL = Device Abstraction Layer

  Provides IoT firmware building blocks for home automation and smart sensors.

  Copyright (C) 2025 Jannik Svensson

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
#include "DeviceUID.h"
#include <cstdio>
#include <cstdint>

#if defined(ESP8266)
#include <esp8266.h>
extern "C" {
#include <user_interface.h>
}

uint64_t getDeviceUID() {
    uint8_t mac[6];
    wifi_get_macaddr(STATION_IF, mac);
    uint64_t id = 0;
    for (int i = 0; i < 6; ++i) id = (id << 8) | mac[i];
    return id;
}

#elif defined(ESP32)
#include <esp_system.h>

uint64_t getDeviceUID() {
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    uint64_t id = 0;
    for (int i = 0; i < 6; ++i) {
        id = (id << 8) | mac[i];
    }
    return id;
}

#else // Desktop fallback
#include <cstring>

#if defined(_WIN32)
#include <windows.h>
#include <iphlpapi.h>
#pragma comment(lib, "iphlpapi.lib")
#elif defined(__APPLE__) || defined(__linux__)
#include <ifaddrs.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cerrno>
#include <cstdio>
#endif

uint64_t getDeviceUID() {
    uint8_t mac[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}; // default dummy UID

#if defined(_WIN32)
    IP_ADAPTER_INFO AdapterInfo[16];
    DWORD dwBufLen = sizeof(AdapterInfo);
    if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == NO_ERROR) {
        memcpy(mac, AdapterInfo[0].Address, 6);
    }

#elif defined(__APPLE__) || defined(__linux__)
    struct ifaddrs *ifap, *ifa;
    if (getifaddrs(&ifap) == 0) {
        for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
            if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_LINK) {
                struct sockaddr_dl* sdl = (struct sockaddr_dl*)ifa->ifa_addr;
                if (sdl->sdl_alen == 6) {
                    memcpy(mac, (uint8_t*)LLADDR(sdl), 6);
                    break;
                }
            }
        }
        freeifaddrs(ifap);
    }
#endif

    uint64_t id = 0;
    for(int i=0;i<6;i++) id = (id<<8)|mac[i];
    return id;
}
#endif

void uidToHex(uint64_t uid, char* buffer) {
    sprintf(buffer, "%012llX", uid);
}