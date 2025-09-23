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

#pragma once

#include <Arduino.h> // Needed for String class

#include <string>
#include <ArduinoJson.h>

#include "HAL_JSON_Display_SSD1306_Element.h"

// Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

namespace HAL_JSON {

    class Display_SSD1306 : public Device {
    private:
        Adafruit_SSD1306* display;

        Display_SSD1306_Element** elements; // TODO change this into plain array for minimal heap fragmentation
        int elementCount;
    public:
        // here we could implement functions for to use with spi interface as well

        static Device* Create(const JsonVariant &jsonObj, const char* type, TwoWire& wire);
        static bool VerifyJSON(const JsonVariant &jsonObj);
        
        Display_SSD1306(const JsonVariant &jsonObj, const char* type, TwoWire& wire);
        ~Display_SSD1306();

        HALOperationResult write(const HALWriteStringRequestValue& val);
        Device* findDevice(UIDPath& path) override;
        void loop() override;

        String ToString() override;
    };
}