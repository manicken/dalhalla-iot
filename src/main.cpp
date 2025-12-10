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

#include "main.h"

#include "HAL_JSON/HAL_JSON_ZeroCopyString.h"
#include "Support/base64.h"

void Timer_SyncTime() {
    DEBUG_UART.println("Timer_SyncTime");
    NTP::NTPConnect();
    tmElements_t now2;
    breakTime(time(nullptr), now2);
    int year = (int)now2.Year + 1970;
    setTime(now2.Hour+1, now2.Minute, now2.Second, now2.Day, now2.Month, year);
}

#if defined(HAL_JSON_H_)
void Alarm_SendToHalCmdExec(const OnTickExtParameters *param)
{
    DEBUG_UART.println("Alarm_SendToHalCmdExec");
    const AsStringParameter* casted_param = static_cast<const AsStringParameter*>(param);
    if (casted_param != nullptr)
    {
        HAL_JSON::ZeroCopyString zcCmd(casted_param->str.c_str());
        std::string dummy;
        HAL_JSON::CommandExecutor::execute(zcCmd,dummy);
    }
}
#endif

Scheduler::NameToFunction nameToFunctionList[] = {
//   name         , onTick            , onTickExt
    {"ntp_sync"   , &Timer_SyncTime   , nullptr           }
#if defined(HAL_JSON_H_)
    ,{"halcmd"     , nullptr           , &Alarm_SendToHalCmdExec}
#endif
};

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

void setup() {
    if (Info::resetReason_is_crash(false)) {
        
        System::failsafeLoop();
    }
    DEBUG_UART.begin(115200);
    DEBUG_UART.setDebugOutput(true);

    DEBUG_UART.println(F("\r\n!!!!!Start of MAIN Setup!!!!!\r\n"));
    Info::PrintHeapInfo();

    DEBUG_UART.println(Info::getResetReasonStr());

    if (LITTLEFS_BEGIN_FUNC_CALL == true) FSBrowser::fsOK = true; // this call is needed before all access to internal Flash file system

    MainConfig::begin(webserver);

#if defined(ESP32) && defined(FSBROWSER_SYNCED_WS_H_)
    if (InitSD_MMC()) FSBrowser::fsOK = true;
#endif
#if defined(USE_DISPLAY)
    init_display();
#endif
    WiFi.setSleep(false);

#ifdef WIFI_MANAGER_WRAPPER_H_
#if defined(USE_DISPLAY)
    bool connected = WiFiManagerWrapper::Setup(display);
#else
    bool connected = WiFiManagerWrapper::Setup();
#endif
#endif
    OTA::setup();

    Scheduler::setup(webserver, nameToFunctionList, sizeof(nameToFunctionList) / sizeof(nameToFunctionList[0]));

    Info::startTime = now();

    System::initWebServerHandlers(webserver);
#ifdef FSBROWSER_SYNCED_WS_H_
    FSBrowser::setup(webserver);
#else
    FSBrowser::setup(webserver);
#endif
    Info::setup(webserver);
    HeartbeatLed::setup(webserver);
#if defined(ESP32)
    System::Start_MDNS();
#endif
    
#if defined(ESP32)
    File test = SD_MMC.open("/StartTimes.log", "a", true);
    test.println(Time_ext::GetTimeAsString(now()).c_str());
    test.close();
#endif

#ifdef HAL_JSON_H_
    HAL_JSON::begin();
#endif
    webserver.begin();

    // make sure that the following are allways at the end of this function
    //Info::PrintHeapInfo();
    DEBUG_UART.println(F("\r\n!!!!!End of MAIN Setup!!!!!\r\n"));
}

void loop() {
    //ArduinoOTA.handle();
    HeartbeatLed::task();
    Scheduler::HandleAlarms();
#ifdef HAL_JSON_H_
    HAL_JSON::loop();
#endif
    
#if defined(ESP8266)
    MDNS.update(); // this is only required on esp8266
#endif
    
#ifdef WIFI_MANAGER_WRAPPER_H_
    WiFiManagerWrapper::Task();
#endif

    if (Serial.available()) {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();
        
        if (strncasecmp(cmd.c_str(), "wifi/", 5) == 0) {
            HAL_JSON::ZeroCopyString zcStr(cmd.c_str());
            zcStr.start+=5; // remove wifi/
            HAL_JSON::ZeroCopyString zcCmd = zcStr.SplitOffHead('/');
            if (zcCmd.EqualsIC("scan")) {
                Serial.println("wifi/scanstart");
                int n = WiFi.scanNetworks();
                for (int i = 0; i < n; i++) {
                    String ssidB64 = b64urlEncode(WiFi.SSID(i).c_str());
                    int freq = WiFi.channel(i) <= 14 ? 2400 : 5000; // crude 2.4/5 GHz
                    int rssi = WiFi.RSSI(i);
                    String enc;
                    switch(WiFi.encryptionType(i)) {
                        case WIFI_AUTH_OPEN: enc = "OPEN"; break;
                        case WIFI_AUTH_WEP: enc = "WEP"; break;
                        case WIFI_AUTH_WPA_PSK: enc = "WPA"; break;
                        case WIFI_AUTH_WPA2_PSK: enc = "WPA2"; break;
                        case WIFI_AUTH_WPA_WPA2_PSK: enc = "WPA/WPA2"; break;
                        case WIFI_AUTH_WPA2_ENTERPRISE: enc = "WPA2-E"; break;
                        default: enc = "UNK"; break;
                    }
                    Serial.println("wifi/ssid/" + ssidB64 + ":" + WiFi.channel(i)+ ":" + String(freq) + ":" + String(rssi) + ":" + enc);
                }
                Serial.println("wifi/scanend");
            } else if (zcCmd.EqualsIC("set")) {
                if (zcStr.CountChar(':') >= 1) {
                    HAL_JSON::ZeroCopyString zcSSID = zcStr.SplitOffHead(':');
                    char ssid[33] = {0};
                    char pass[65] = {0};
                    int ssidLen = b64urlDecode((uint8_t*)ssid, zcSSID.ToString().c_str());
                    int passLen = b64urlDecode((uint8_t*)pass, zcStr.ToString().c_str());
                    WiFi.persistent(true);      // ESP8266: saves credentials to flash. ESP32: harmless, ignored.
                    WiFi.begin(ssid, pass);     // Connects to the AP.
                    WiFi.setAutoConnect(true);  // ESP32: ensures reconnect on boot. ESP8266: also works.
                    WiFi.setAutoReconnect(true);// ESP32: reconnect if connection drops. ESP8266: ignored (does nothing).
                    // Optionally wait a few milliseconds to ensure settings are written
                    delay(1000); 
                    Serial.println("wifi/set/OK");
                    // Restart the device so it boots with the saved credentials
                    ESP.restart();
                } else {
                    Serial.println("wifi/set/error/missingparams");
                }
            }
        } else if (strncasecmp(cmd.c_str(), "hal/", 4) == 0) {
            HAL_JSON::ZeroCopyString zcStr(cmd.c_str());
            zcStr.start+=4; // remove hal/
            std::string msg;
            HAL_JSON::CommandExecutor::execute(zcStr, msg);
            Serial.println(msg.c_str());
        } else {
            Serial.println("error/cmd/unknown");
        }
    }
}

#if defined(USE_DISPLAY)
void init_display(void)
{
    delay(1000);
    Wire.begin(21, 22, 100000); // SDA=21, SCL=22
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
