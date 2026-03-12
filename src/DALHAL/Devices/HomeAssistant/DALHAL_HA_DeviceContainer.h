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

#include <DALHAL/Devices/HomeAssistant/DALHAL_HA_DeviceTypeReg.h>

#include <Arduino.h> // Needed for String class

#include <string>
#include <ArduinoJson.h>
#include <PubSubClient.h> // uses a modded PubSubClient placed in /lib folder

#include <DALHAL/Core/Device/DALHAL_Device.h>


namespace DALHAL {

    class HA_DeviceContainer : public Device {
    private:
        Device** devices;
        int deviceCount;
    public:

        
        /** called regulary from the main loop */
        void loop() override;
        /** called when all hal devices has been loaded */
        void begin() override;
        /** used to find sub/leaf devices @ "group devices" */
        DeviceFindResult findDevice(UIDPath& path, Device*& outDevice) override;

        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(const JsonVariant& jsonObj, const char* type, void* context);
        static constexpr Registry::Define RegistryDefine = {
            Registry::UseRootUID::Void,
            Create,
            VerifyJSON
        };
        HA_DeviceContainer(const JsonVariant& jsonObj, const char* type, HA_CreateFunctionContext* context);
        ~HA_DeviceContainer();
        String ToString() override;
    };
}