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
#include "WifiManagerWrapper.h"
#include "../Support/NTP.h"

#if defined(USE_DISPLAY)
// Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#endif

namespace WiFiManagerWrapper {

    WiFiManager wifiManager;
    bool portalRunning = false;

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
            wifiManager.setConfigPortalBlocking(false);
            wifiManager.startConfigPortal();
            portalRunning = true;
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

        return wifiConnected;
    }


    void startPortalNonBlocking() {
        wifiManager.setConfigPortalBlocking(false);
        wifiManager.startConfigPortal();
        portalRunning = true;
    }

    void Task() {
        // Automatic reconnect
        static unsigned long lastReconnectAttempt = 0;
        if (WiFi.status() != WL_CONNECTED) {
            if (millis() - lastReconnectAttempt > 10000) { // every 10 seconds
                // request reconnect (non-blocking, connection happens in background)
                if (WiFi.reconnect())
                    Serial.println("WiFi lost, trying reconnect...");
                else
                    Serial.println("WiFi lost, reconnect could not be started...");
                lastReconnectAttempt = millis();
                if (portalRunning == false)
                    startPortalNonBlocking();
            }
        } else {
            if (portalRunning) {
                portalRunning = false;
                NTP::NTPConnect();
            }
        }
        if (portalRunning)
            wifiManager.process();
    }
}
