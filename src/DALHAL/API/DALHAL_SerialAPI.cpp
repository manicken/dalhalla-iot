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

#include "DALHAL_SerialAPI.h"

#include <DALHAL/Core/Types/DALHAL_ZeroCopyString.h>
#include <DALHAL/API/DALHAL_CommandExecutor.h>
#include <System/Platform/Platform.h>

#include <Arduino.h> // Serial (Arduino core subset)
#include <string>

// longest command line accepted over serial, longer lines are discarded
#define DALHAL_SERIAL_API_MAX_LINE_LENGTH 1024

namespace DALHAL {

// on PC stdin is read by the command loop thread in pc_simulation/commandLoop.cpp
#if !defined(DALHAL_BACKEND_PC)

    static std::string lineBuffer;
    static bool lineOverflow = false;

    static bool SendResponse(const ZeroCopyString& zcMsg, CmdCbType type) {
        (void)type; // ignore type for now until we implemented a real protocol over serial, maybe similar to the WebSocket one
        if (zcMsg.IsFlash()) { // only possible on ESP8266
            Serial.print(reinterpret_cast<const __FlashStringHelper*>(zcMsg.start));
            Serial.print("\r\n");
        } else {
            printf("%.*s\r\n", (int)zcMsg.Length(), zcMsg.start);
        }
        return true;
    }

    static void ExecuteLine() {
        ZeroCopyString zcStrCmd(lineBuffer.c_str());
        zcStrCmd.Trim();
        if (zcStrCmd.NotEmpty()) {
            CommandExecutor::execute(zcStrCmd, SendResponse);
        }
    }

    /*static*/ void SerialAPI::loop() {
        // non blocking: takes what has arrived, executes when a full line is received
        while (Serial.available()) {
            int c = Serial.read();
            if (c < 0) break;
            if (c == '\r') continue;
            if (c == '\n') {
                if (lineOverflow) {
                    printf("SerialAPI: line longer than %u chars, discarded\r\n", (unsigned)DALHAL_SERIAL_API_MAX_LINE_LENGTH);
                } else {
                    ExecuteLine();
                }
                lineBuffer.clear();
                lineOverflow = false;
                continue;
            }
            if (lineBuffer.size() < DALHAL_SERIAL_API_MAX_LINE_LENGTH) {
                lineBuffer += (char)c;
            } else {
                lineOverflow = true;
            }
        }
    }

#else

    /*static*/ void SerialAPI::loop() {}

#endif

}
