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

#if defined(ESP32) || defined(ESP8266)

#include "WiFiLink.h"

#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include <System/WiFiConnectionManager.h>

namespace Network {
namespace WiFiLink {

    void PreInit() {
        WiFi.setAutoReconnect(true);
        WiFi.persistent(true);
        WiFi.setSleep(false);
    }

    void Begin() {
        WiFiConnectionManager::init(
            DALHAL_WIFI_PORTAL_AP_SSID,
            DALHAL_WIFI_PORTAL_AP_PASS,          // AP Password (min 8 chars)
            DALHAL_WIFI_STA_TIMEOUT_MS,          // STA connection timeout
            DALHAL_WIFI_PORTAL_AP_TIMEOUT_MS,    // AP timeout before reconnect attempts
            DALHAL_WIFI_RECONNECT_INTERVAL_MS    // Reconnect interval
        );
        WiFiConnectionManager::WaitForConnectionUntilTimeout();
    }

    void Loop() {
        WiFiConnectionManager::task();
    }

    void BeginFailsafe(uint32_t staConnectTimeoutMs) {
        // === STEP 0: WiFi Preparation ===
        WiFi.setSleep(false);
        WiFi.disconnect(true);  // Turn off previous connection completely
        delay(100);
        WiFi.mode(WIFI_AP_STA);

        // Setup AP (failsafe access point)
        if (!WiFi.softAP(DALHAL_FAILSAFE_AP_SSID, DALHAL_FAILSAFE_AP_PASS)) {
            Serial.println(F("ERROR: Failed to start AP!"));
        } else {
            WiFi.softAP(DALHAL_FAILSAFE_AP_SSID, DALHAL_FAILSAFE_AP_PASS);
            Serial.print(F("AP credentials: SSID=" DALHAL_FAILSAFE_AP_SSID ", PASS=" DALHAL_FAILSAFE_AP_PASS));
            Serial.print(F("AP IP: "));
            Serial.println(WiFi.softAPIP());
        }

        // --- STEP 1: Connect to last known WiFi (non-blocking with timeout) ---
        Serial.println(F("\n[WiFi] Attempting to connect to last known network..."));
        WiFi.begin();  // Use stored credentials

        unsigned long wifiConnectTimeout = millis() + staConnectTimeoutMs;
        bool wifiConnected = false;
        int wifiAttempts = 0;

        while (millis() < wifiConnectTimeout && !wifiConnected) {
            delay(500);
            wifiAttempts++;

            if (WiFi.status() == WL_CONNECTED) {
                wifiConnected = true;
                Serial.println(F("\n[WiFi] ✓ Connected to WiFi!"));
                Serial.print(F("[WiFi] IP: "));
                Serial.println(WiFi.localIP());
                break;
            }

            if (wifiAttempts % 4 == 0) {  // Print every 2 seconds
                Serial.print(F("."));
            }
        }

        if (!wifiConnected) {
            Serial.println(F("\n[WiFi] ✗ Failed to connect to stored network (continuing in AP-only mode)"));
        }
    }

    void LoopFailsafe() {
        static unsigned long lastReconnect = 0;
        if (WiFi.status() != WL_CONNECTED && millis() - lastReconnect > 5000) {
            WiFi.reconnect();
            lastReconnect = millis();
        }
    }

    bool IsUp() {
        return WiFi.status() == WL_CONNECTED;
    }

    IPAddress LocalIP() {
        return WiFi.localIP();
    }

    bool HostByName(const char* host, IPAddress& out) {
        return WiFi.hostByName(host, out) == 1;
    }

}
}

#endif
