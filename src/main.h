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

#define USE_DISPLAY

// basic
#include <EEPROM.h>
#include "SPI.h"


// WiFi
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif

#include "Support/Logger.h"

// OTA
#include "System/OTA.h"

// HTTP stuff
#include <ESPAsyncWebServer.h>

#if defined(USE_DISPLAY)
// Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#endif

// other addons

#include <LittleFS.h>

//#define ARDUINOJSON_ENABLE_PROGMEM 0
#include <TimeLib.h>
#include <TimeAlarms.h>
#include "Scheduler/Scheduler.h"

#include "Support/Info.h"

#include "Support/Time_ext.h"


#include "System/MainConfig.h"
#include "System/FSBrowserAsync.h"

#if defined(ESP8266)
#include <ESP8266mDNS.h>
#elif defined(ESP32)
#include <ESPmDNS.h>
#endif

#include "HAL_JSON/HAL_JSON.h"
#ifdef HAL_JSON_H_
#include "HAL_JSON/ScriptEngine/HAL_JSON_SCRIPT_ENGINE.h"
#endif

#include "HAL_JSON/HAL_JSON_CommandExecutor.h"

#include "System/WiFiManagerWrapper.h"
#include "System/System.h"
#include "Support/ConstantStrings.h"
#include "Drivers/HearbeatLed.h" // this should not be here in final version (should only be accessible through HAL interface)

#include "esp_task_wdt.h"

#define MAIN_URLS_JSON_CMD              F("/json_cmd")


#if defined(ESP8266)
    #define DEBUG_UART Serial1
#elif defined(ESP32)
    #define DEBUG_UART Serial
#endif

//TCP2UART tcp2uart;

// QUICKFIX...See https://github.com/esp8266/Arduino/issues/263
#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

#define WIFI_TIMEOUT 30000              // checks WiFi every ...ms. Reset after this time, if WiFi cannot reconnect.
#define HTTP_PORT 80

AsyncWebServer webserver(HTTP_PORT);

#ifdef ESP8266
#define LITTLEFS_BEGIN_FUNC_CALL LittleFS.begin()
#elif ESP32
#define AUTOFORMAT_ON_FAIL true
#define LITTLEFS_BEGIN_FUNC_CALL LittleFS.begin(AUTOFORMAT_ON_FAIL, "/LittleFS", 10, "spiffs")
#endif

unsigned long currTime = 0;

bool wifiConnected = false;
bool portalRequested = false;

#if defined(USE_DISPLAY)
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(128, 64, &Wire, -1); // -1 = no reset pin
void init_display(void);
#endif