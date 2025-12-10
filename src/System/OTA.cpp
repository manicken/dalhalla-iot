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

#include "OTA.h"

namespace OTA
{
    WiFiClient wifiClient;
    
    static int otaPartProcentCount = 0;
    
    void setup()
    {
        //Pulled_check(wifiClient);
        setup_PushedOTA();
    }

    void Download_Update(String url)
    {
        Download_Update(url.c_str());
    }
    void Download_Update(const char *url)
    {
        //EEPROM.put(SPI_FLASH_SEC_SIZE, "hello");
#if defined (ESP8266)
        DEBUG_UART.println(F("checking for updates"));
        //String updateUrl = "http://espOtaServer/esp_ota/" + String(ESP.getChipId(), HEX) + ".bin";
        t_httpUpdate_return ret = ESPhttpUpdate.update(wifiClient, url);
        DEBUG_UART.print(F("HTTP_UPDATE_"));
        switch (ret) {
        case HTTP_UPDATE_FAILED:
            DEBUG_UART.println(F("FAIL Error "));
            DEBUG_UART.printf("(%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
            break;

        case HTTP_UPDATE_NO_UPDATES:
            DEBUG_UART.println(F("NO_UPDATES"));
            break;

        case HTTP_UPDATE_OK:
            DEBUG_UART.println(F("OK"));
            break;
        }
#endif
    }

    void setup_PushedOTA()
    {
        ArduinoOTA.onStart([]() {
            otaPartProcentCount = 0;
            String type;

            if (ArduinoOTA.getCommand() == U_FLASH) {

            type = "sketch";

            } else { // U_SPIFFS

            type = "filesystem";

            }



            // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()

            DEBUG_UART.println(F("OTA Start\rOTA Progress: "));

        });

        ArduinoOTA.onEnd([]() {

            DEBUG_UART.println("\n100%\nOTA End");

        });
/*
        ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {

            //DEBUG_UART.printf("Progress: %u%%\r", (progress / (total / 100)));
            //Serial1.printf("%u%%\r", (progress / (total / 100)));
            //DEBUG_UART.print("-");
            if (otaPartProcentCount < 9)
            otaPartProcentCount++;
            else
            {
            otaPartProcentCount = 0;
            DEBUG_UART.printf(" %u%%\r", (progress / (total / 100)));
            }
        });
        */

        ArduinoOTA.onError([](ota_error_t error) {
            DEBUG_UART.printf("OTA Error");
            DEBUG_UART.printf("[%u]: ", error);
            if (error == OTA_AUTH_ERROR) DEBUG_UART.println(F("Auth Failed"));
            else if (error == OTA_BEGIN_ERROR) DEBUG_UART.println(F("Begin Failed"));
            else if (error == OTA_CONNECT_ERROR) DEBUG_UART.println(F("Connect Failed"));
            else if (error == OTA_RECEIVE_ERROR) DEBUG_UART.println(F("Receive Failed"));
            else if (error == OTA_END_ERROR) DEBUG_UART.println(F("End Failed"));
        });

        ArduinoOTA.begin();
        //WiFi.setHostname("test");

        DEBUG_UART.println("Ready");

        DEBUG_UART.print("IP address: ");

        DEBUG_UART.println(WiFi.localIP());
    }
}