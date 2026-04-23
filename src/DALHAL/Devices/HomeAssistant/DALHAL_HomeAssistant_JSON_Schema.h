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

#include <ArduinoJson.h>

namespace DALHAL {

    // forward declarations
    class HomeAssistant; 
    struct DeviceCreateContext;
    struct HA_CreateFunctionContext;
    class Device;

    namespace JsonSchema {

        // forward declaration
        struct JsonObjectSchema; 

        namespace HomeAssistant {

            extern const JsonObjectSchema Root;

            struct Extractors final {
                static void CreateDevicesFromItems(const JsonArray& items, DALHAL::HA_CreateFunctionContext& createFuncContext, DALHAL::Device** devices, int& index);
                static void ExtractGlobalGroupMode(const DALHAL::DeviceCreateContext& context, void* out);
                static void ExtractIndividualGroupMode(const DALHAL::DeviceCreateContext& context, void* out);

                /** used by the device class */
                static void Apply(const DALHAL::DeviceCreateContext& context, DALHAL::HomeAssistant* out);
            };

        }

    }

}