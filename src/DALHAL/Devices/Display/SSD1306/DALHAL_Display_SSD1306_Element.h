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

#pragma once

#include <Arduino.h> // Needed for String class

#include <string>

#include <DALHAL/Core/Device/DALHAL_Device.h>
#include <DALHAL/Core/Device/DALHAL_CachedDeviceAccess.h>
#include <DALHAL/Core/Types/DALHAL_Registry.h>

namespace DALHAL {

    namespace JsonSchema { namespace Display_SSD1306_Element { struct Extractors; } } // forward declaration

    class Display_SSD1306_Element : public Device {
        friend struct JsonSchema::Display_SSD1306_Element::Extractors; // allow access to private memebers of this class from the schema extractor

    public:
        CachedDeviceAccess* cdaSource;
        HALValue val;
        std::string label;
        uint8_t xPos;
        uint8_t yPos;

        Display_SSD1306_Element(Display_SSD1306_Element&) = delete;
        Display_SSD1306_Element(DeviceCreateContext& context);
        ~Display_SSD1306_Element() override = default;

        HALOperationResult write(const HALValue& val) override;

        String ToString() override;
    };
}