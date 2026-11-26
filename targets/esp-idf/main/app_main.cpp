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

// DALHAL on plain ESP-IDF: config, script engine, command API over the serial console and the network.
// The same order as setup()/loop() in src/main.cpp (Arduino) for the parts that exist here.

#include <Arduino.h>
#include <LittleFS.h>

#include <DALHAL/DALHAL.h>
#include <DALHAL/Core/Manager/DALHAL_GPIO_Manager.h>
#include <DALHAL/Drivers/HearbeatLed.h>
#include <Scheduler/Scheduler.h>
#include <System/Network/Network.h>
#include <Support/NTP.h>
#include <DALHAL/API/WebSocket/DALHAL_WebSocketAPI_Backend.h>
#include <ArduinoOTA.h>
#include <System/Platform/Platform.h>
#include "failsafe.h"

/** link independent: called on every Up/Down transition of the network, as in src/main.cpp */
static void onNetworkEvent(Network::Event event) {
    if (event != Network::Event::Up) {
        printf("Network down\r\n");
        return;
    }
    printf("Network up, IP: %s\r\n", Network::LocalIP().toString().c_str());
    DALHAL::WebSocketAPI::setup(); // already started by DALHAL::begin(), keeps the running server
    ArduinoOTA.begin(); // espota protocol, "pio run -t upload" with the IP as upload_port
    NTP::NTPConnect();
}

extern "C" void app_main(void)
{
    Network::PreInit();
    DALHAL::GPIO_manager::triStateAvailablePins();

    // after a crash only the ways in are started, not DALHAL (as the Arduino target)
    if (Platform::ResetWasCrash(true)) {
        FailsafeLoop();
    }

    printf("\r\n!!!!!Start of MAIN Setup (ESP-IDF)!!!!!\r\n");

    // same partition and mount point as the Arduino builds, so the same LittleFS image works
    if (!LittleFS.begin(false)) {
        printf("LittleFS mount failed, upload the filesystem image (data/ folder)\r\n");
    }

    // WiFi credentials are the ones stored in NVS (shared with the Arduino firmware)
    Network::SetEventCallback(onNetworkEvent);
    Network::Begin();

    HeartbeatLed::setup();
    DALHAL::begin(); // config, devices, scripts

    printf("\r\n!!!!!End of MAIN Setup!!!!!\r\n");

    while (true) {
        HeartbeatLed::task();
        Scheduler::HandleAlarms();
        DALHAL::loop(); // command queue, devices, script engine, SerialAPI
        Network::Loop();
        ArduinoOTA.handle();
        // lets the idle task run so the task watchdog is fed, 1 ms with CONFIG_FREERTOS_HZ=1000
        vTaskDelay(1);
    }
}
