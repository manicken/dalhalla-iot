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

#include "main.h"

#ifdef DALHAL_H_
#include <DALHAL/Core/Types/DALHAL_ZeroCopyString.h>
#include <DALHAL/Core/Manager/DALHAL_GPIO_Manager.h>
#endif

#if defined(ESP32)
#include <SD_MMC.h>
#endif

#include <System/WiFiConnectionManager.h>
#include <DALHAL/API/DALHAL_WebSocketAPI.h>

#define BUILD_VER "1.0"

/*void Timer_SyncTime() {
    DEBUG_UART.println("Timer_SyncTime");
    NTP::NTPConnect();
    tmElements_t now2;
    breakTime(time(nullptr), now2);
    int year = (int)now2.Year + 1970;
    setTime(now2.Hour+1, now2.Minute, now2.Second, now2.Day, now2.Month, year);
}

#if defined(DALHAL_H_)
void Alarm_SendToHalCmdExec(const OnTickExtParameters *param)
{
    DEBUG_UART.println("Alarm_SendToHalCmdExec");
    const AsStringParameter* casted_param = static_cast<const AsStringParameter*>(param);
    if (casted_param != nullptr)
    {
        DALHAL::ZeroCopyString zcCmd(casted_param->str.c_str());
        //std::string dummy;
        DALHAL::CommandExecutor::execute(zcCmd,nullptr);
    }
}
#endif

Scheduler::NameToFunction nameToFunctionList[] = {
//   name         , onTick            , onTickExt
    {"ntp_sync"   , &Timer_SyncTime   , nullptr           }
#if defined(DALHAL_H_)
    ,{"halcmd"     , nullptr           , &Alarm_SendToHalCmdExec}
#endif
};

*/
#ifdef WIFI_CONNECTION_MANAGER_H
void onWiFiEvent(WiFiConnectionManager::Event event, const char* info) {
    switch (event) {
        case WiFiConnectionManager::Event::STA_CONNECT_SUCCESS:
            Serial.print(F("✓ WiFi connected: ")); Serial.println(info);
            Serial.println(F("wifi/status/ok/connect"));
#if defined(USE_DISPLAY)
            display.setCursor(0, 9);
            display.println(F("WiFi connected"));
            display.setCursor(0, 17);
            display.println(WiFi.localIP());
            display.display();
            delay(2000); // allow user to see result
#endif
            // You could trigger WS API initialization here
            DALHAL::WebSocketAPI::setup();
            break;

        case WiFiConnectionManager::Event::STA_CONNECT_FAILED:
            Serial.print(F("✗ WiFi failed: ")); Serial.println(info);
            Serial.println(F("wifi/status/error/connect"));
#if defined(USE_DISPLAY)
            display.setCursor(0, 9);
            display.println(F("WiFi FAIL"));
            display.display();
            delay(2000); // allow user to see result
#endif
            // Fallback to local AP configuration
            break;

        case WiFiConnectionManager::Event::AP_STARTED:
            Serial.print(F("★ AP mode active: ")); Serial.println(info);
#if defined(USE_DISPLAY)
            display.setCursor(0, 9);
            display.println(F("AP mode active"));
            display.display();
            delay(2000); // allow user to see result
#endif
            // Start your configuration web server here
            break;

        case WiFiConnectionManager::Event::AP_TO_STA_SUCCESS:
            Serial.print(F("✓ Reconnected from AP:")); Serial.println(info);
#if defined(USE_DISPLAY)
            display.setCursor(0, 9);
            display.println(F("Reconnected from AP"));
            display.display();
            delay(2000); // allow user to see result
#endif
            break;

        case WiFiConnectionManager::Event::RECONNECT_ATTEMPT:
            Serial.println(F("↻ Attempting WiFi reconnection from AP..."));
#if defined(USE_DISPLAY)
            display.setCursor(0, 9);
            display.println(F("Attempting WiFi reconnection from AP"));
            display.display();
            delay(2000); // allow user to see result
#endif
            break;
    }
}
#endif

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

void setup() {
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
    WiFi.setSleep(false);
#ifdef DALHAL_H_
    DALHAL::GPIO_manager::triStateAvailablePins();
#endif

    if (Info::resetReason_is_crash(true)) {
        System::failsafeLoop();
    }
    DEBUG_UART.begin(115200);
    DEBUG_UART.setDebugOutput(true);

    DEBUG_UART.println(F("\r\n!!!!!Start of MAIN Setup!!!!!\r\n"));
    Info::PrintHeapInfo();

    DEBUG_UART.println(Info::getResetReasonStr());

    System::Setup(); // only littlefs init

#ifdef WIFI_CONNECTION_MANAGER_H
    WiFiConnectionManager::init(
        "DALHAL-Failsafe",             // AP SSID
        "",                            // AP Password (min 8 chars)
        8000,                          // STA connection timeout: 8 seconds
        60000,                         // AP timeout before reconnect attempts: 60 seconds
        30000                          // Reconnect interval: 30 seconds
    );
    // Set event callback (optional but recommended)
    WiFiConnectionManager::setEventCallback(onWiFiEvent);
#endif
    Serial.println(F("Setup complete - WiFi manager initialized"));
#if defined(USE_DISPLAY)
    init_display();
#endif
#if defined(USE_DISPLAY)
    display.setCursor(0, 0);
    display.println(F("WiFi connecting..."));
    display.display();
#endif

#if defined(ESP32) && defined(FSBROWSER_SYNCED_WS_H_)
    if (InitSD_MMC()) FSBrowser::fsOK = true;
#endif

    OTA::setup();
    //Scheduler::setup(nameToFunctionList, sizeof(nameToFunctionList) / sizeof(nameToFunctionList[0]));
    Info::startTime = now();
    HeartbeatLed::setup();
    
#if defined(ESP32) && !defined(esp32c3)
    File test = SD_MMC.open("/StartTimes.log", "a", true);
    test.println(Time_ext::GetTimeAsString(now()).c_str());
    test.close();
#endif
    System::initWebServerHandlers(webserver);
#ifdef DALHAL_H_
    DALHAL::begin();
#endif
    webserver.begin();

    // make sure that the following are allways at the end of this function
    Info::PrintHeapInfo();
    DEBUG_UART.println(F("\r\n!!!!!End of MAIN Setup!!!!!\r\n"));
    delay(0);
}

void loop() {
    ArduinoOTA.handle();
    HeartbeatLed::task();
    //Scheduler::HandleAlarms();
#ifdef DALHAL_H_
    DALHAL::loop();
#endif
#ifdef WIFI_CONNECTION_MANAGER_H
    WiFiConnectionManager::task();
#endif

}

#if defined(USE_DISPLAY)
void init_display(void)
{
    delay(1000);
#if defined(ESP32)
    Wire.begin(21, 22, 100000); // SDA=21, SCL=22
#elif defined(ESP8266)
    Wire.begin(4, 5); // SDA=21, SCL=22
#endif
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        DEBUG_UART.println(F("OLED init fail"));
        return;
    }

    DEBUG_UART.println(F("OLED OK"));
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);
    //display.println(F("Hello ESP32!"));
    display.display(); // <--- push buffer to screen
}
#endif


#endif