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

#include "WiFiManager.h"
#include "WiFiManagerWrapper.h"
#include "../Support/NTP.h"

#if defined(USE_DISPLAY)
// Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#endif

#if defined(ESP32)
#include <WiFi.h>
wifi_err_reason_t lastReason = WIFI_REASON_UNSPECIFIED;

void WiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info) {
    if (event == SYSTEM_EVENT_STA_DISCONNECTED) {
        lastReason = (wifi_err_reason_t)info.wifi_sta_disconnected.reason;
        Serial.printf("[ESP32] Disconnect reason: %d\n", lastReason);
    }
}

#elif defined(ESP8266)
#include <ESP8266WiFi.h>
uint8_t lastReason = 0;

void onWiFiDisconnect(const WiFiEventStationModeDisconnected& event) {
    lastReason = event.reason;
    Serial.printf("[ESP8266] Disconnect reason: %d\n", lastReason);
}
#endif

void initWiFiEvents() {
#if defined(ESP32)
    WiFi.onEvent(WiFiEvent);

#elif defined(ESP8266)
    static WiFiEventHandler handler;
    handler = WiFi.onStationModeDisconnected(onWiFiDisconnect);
#endif
}

bool isBadCredentials() {
#if defined(ESP32)
    return (lastReason == WIFI_REASON_AUTH_FAIL ||
            /*lastReason == WIFI_REASON_PASSWORD_ERROR ||*/
            lastReason == WIFI_REASON_HANDSHAKE_TIMEOUT);

#elif defined(ESP8266)
    // ESP8266 reason codes differ (15 = AUTH_FAIL)
    return (lastReason == REASON_AUTH_FAIL ||
            lastReason == REASON_MIC_FAILURE);
#endif
}

namespace WiFiManagerWrapper {

    WiFiManager wifiManager;
    /** both flags that the portal is running and also that the wifi connection was lost */
    bool portalRunning = false;
    unsigned long lastReconnect = 0;
    unsigned long lastPortalStart = 0;

    const unsigned long RECONNECT_INTERVAL = 10000;   // 10s
    const unsigned long PORTAL_COOLDOWN    = 600000;  // 10 minutes

    void startPortalNonBlocking() {
        wifiManager.setConfigPortalBlocking(false);
        wifiManager.startConfigPortal();
        portalRunning = true;
    }

#if defined(USE_DISPLAY)
    bool Setup(Adafruit_SSD1306& display) {
#else
    bool Setup() {
#endif
        Serial.println(F("wifi/status/connecting"));

#if defined(USE_DISPLAY)
        display.setCursor(0, 0);
        display.println(F("WiFi connecting..."));
        display.display();
#endif

        // Try connecting to saved Wi-Fi with a short timeout (5 seconds)
        WiFi.persistent(true);
        WiFi.begin(); // Use saved credentials

        bool wifiConnected = false;
        unsigned long start = millis();
        const unsigned long connectTimeout = 5000; // ms

        while (WiFi.status() != WL_CONNECTED && millis() - start < connectTimeout) {
            delay(50); // short delay to avoid blocking too long
            yield();
        }

        wifiConnected = (WiFi.status() == WL_CONNECTED);

        // If not connected, start non-blocking portal immediately
        if (!wifiConnected) {
            Serial.println(F("wifi/status/error/connect")); // send structured error for easy parse
            startPortalNonBlocking();
        } else {
            Serial.println(F("wifi/status/ok")); // send structured error for easy parse
        }

#if defined(USE_DISPLAY)
        display.setCursor(0, 9);
        if (wifiConnected) {
            display.println(F("OK"));
            display.setCursor(0, 17);
            display.println(WiFi.localIP());
        } else {
            display.println(F("FAIL"));
        }
        display.display();
        delay(2000); // allow user to see result
#endif
        initWiFiEvents();

        return wifiConnected;
    }

    void Task() {

        // ---- WiFi online ----
        if (WiFi.status() == WL_CONNECTED) {
            if (portalRunning) {
                Serial.println("WiFi connected, stopping portal...");
                portalRunning = false;
                // wifiManager.stopConfigPortal(); // only if your WM version supports it
            }
            return;
        }

        // ---- WiFi offline ----
        if (millis() - lastReconnect < RECONNECT_INTERVAL)
            return;

        lastReconnect = millis();

        // Try non-blocking reconnect
        if (WiFi.reconnect()) {
            Serial.println("Trying WiFi reconnect...");
        } else {
            Serial.println("Reconnect could not be started");
        }


        // ---- If credentials are bad, start the portal ----
        if (isBadCredentials()) {

            // prevent repeated portal spam
            if (!portalRunning && millis() - lastPortalStart > PORTAL_COOLDOWN) {

                Serial.println("Bad credentials detected, starting portal...");
                startPortalNonBlocking();

                portalRunning = true;
                lastPortalStart = millis();
            }
        }
        else {
            // router down? keep retrying indefinitely
            Serial.println("AP not found or temp issue, retrying...");
        }

        if (portalRunning) {
            wifiManager.process(); // non-blocking
        }
    }

}
