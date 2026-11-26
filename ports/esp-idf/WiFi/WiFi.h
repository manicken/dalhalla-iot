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
  The part of the arduino-esp32 WiFi API that DALHAL uses, on esp_wifi + esp_netif.
  Credentials are kept in esp_wifi's own NVS storage, the same as arduino-esp32 with WiFi.persistent(true),
  so credentials saved by the Arduino firmware are used by this one and the other way around.
*/

#pragma once

#include <cstdint>
#include <vector>
#include <WString.h>
#include <IPAddress.h>
#include <WiFiClient.h>
#include "esp_wifi_types.h"

// Arduino names for the esp_wifi modes
#define WIFI_OFF    WIFI_MODE_NULL
#define WIFI_STA    WIFI_MODE_STA
#define WIFI_AP     WIFI_MODE_AP
#define WIFI_AP_STA WIFI_MODE_APSTA

typedef enum {
    WL_NO_SHIELD = 255,
    WL_IDLE_STATUS = 0,
    WL_NO_SSID_AVAIL = 1,
    WL_SCAN_COMPLETED = 2,
    WL_CONNECTED = 3,
    WL_CONNECT_FAILED = 4,
    WL_CONNECTION_LOST = 5,
    WL_DISCONNECTED = 6
} wl_status_t;

#define WIFI_SCAN_FAILED (-2)

class WiFiClass {
public:
    bool mode(wifi_mode_t m);
    wifi_mode_t getMode();

    /** connects with the stored credentials */
    wl_status_t begin();
    /** stores the credentials (when persistent) and connects */
    wl_status_t begin(const char* ssid, const char* passphrase = nullptr);
    bool reconnect();
    bool disconnect(bool wifioff = false, bool eraseap = false);
    wl_status_t status();

    bool setAutoReconnect(bool enable) { autoReconnect = enable; return true; }
    bool setAutoConnect(bool enable) { (void)enable; return true; } // esp_wifi has no auto connect, begin() is always called
    void persistent(bool enable);
    bool setSleep(bool enable);
    bool setHostname(const char* name);

    IPAddress localIP();
    String SSID();
    int8_t RSSI();

    bool softAP(const char* ssid, const char* passphrase = nullptr, int channel = 1, int ssidHidden = 0, int maxConnection = 4);
    IPAddress softAPIP();

    int hostByName(const char* host, IPAddress& result);

    /** blocking scan, returns the number of networks or WIFI_SCAN_FAILED */
    int16_t scanNetworks(bool async = false, bool showHidden = false);
    String SSID(uint8_t index);
    int32_t RSSI(uint8_t index);
    int32_t channel(uint8_t index);
    wifi_auth_mode_t encryptionType(uint8_t index);

private:
    bool init();
    bool ensureStarted();
    bool addMode(wifi_mode_t m);

    bool initialized = false;
    bool started = false;
    bool autoReconnect = true;
    bool persistentStorage = true;
    std::string hostname;
    std::vector<wifi_ap_record_t> scanResults;
};

extern WiFiClass WiFi;
