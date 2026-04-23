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

#include <DALHAL/Devices/HomeAssistant/DALHAL_HA_DeviceTypeReg.h>

#include <Arduino.h> // Needed for String class

#include <string>
#include <ArduinoJson.h>
#include <PubSubClient.h> // uses a modded PubSubClient placed in /lib folder

#include <DALHAL/Core/Device/DALHAL_Device.h>

#include <DALHAL/Devices/HomeAssistant/DALHAL_HA_CreateFunctionContext.h>

namespace DALHAL {

    namespace JsonSchema { namespace HA_DeviceContainer { struct Extractors; } } // forward declaration

    class HA_DeviceContainer : public Device {
        friend struct JsonSchema::HA_DeviceContainer::Extractors; // allow access to private memebers of this class from the schema extractor

    public: // public static fields and exposed external structures
        static const Registry::DefineBase RegistryDefine;
        static Device* Create(DeviceCreateContext& context);

    private:
        Device** devices;
        int deviceCount;

    public:
        HA_DeviceContainer(HA_CreateFunctionContext& context);
        ~HA_DeviceContainer() override;

        void begin() override;
        void loop() override;

        DeviceFindResult findDevice(UIDPath& path, Device*& outDevice) override;

        String ToString() override;
    };
}