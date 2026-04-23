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

#include "DALHAL_HA_DeviceContainer.h"

#include "DALHAL_HA_CreateFunctionContext.h"

#include <DALHAL/Core/Device/DALHAL_Device.h>

//#include <DALHAL/Devices/HomeAssistant/DALHAL_HomeAssistant.h>

#include <DALHAL/Support/DALHAL_Logger.h>

#include <DALHAL/Devices/HomeAssistant/DALHAL_HA_CreateFunctionContext.h>

#include "DALHAL_HA_DeviceContainer_JSON_Schema.h"


namespace DALHAL {
    constexpr Registry::DefineBase HA_DeviceContainer::RegistryDefine = {
        Create,
        &JsonSchema::HA_DeviceContainer::Root,
    };

    Device* HA_DeviceContainer::Create(DeviceCreateContext& context) {
        return new HA_DeviceContainer(static_cast<HA_CreateFunctionContext&>(context));
    }
    
    HA_DeviceContainer::HA_DeviceContainer(HA_CreateFunctionContext& context) : Device(context.deviceType) {
        JsonSchema::HA_DeviceContainer::Extractors::Apply(context, this);
    }

    HA_DeviceContainer::~HA_DeviceContainer() {
        if (devices) {
            for (int i = 0; i < deviceCount; ++i) {
                delete devices[i];
            }
            delete[] devices;
        }
    }

    void HA_DeviceContainer::begin() {
        if (devices == nullptr || deviceCount == 0) return;
        for (int i=0;i<deviceCount;i++)
        {
            devices[i]->begin();
        }
    }

    void HA_DeviceContainer::loop() {
        if (devices == nullptr || deviceCount == 0) return;
        for (int i=0;i<deviceCount;i++)
        {
            devices[i]->loop();
        }
    }

    DeviceFindResult HA_DeviceContainer::findDevice(UIDPath& path, Device*& outDevice) {
        return Device::findInArray(devices, deviceCount, path, this, outDevice);
    }

    String HA_DeviceContainer::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += this->Type;
        ret += "\",\"items\":[";
        // TODO fix the following as it wont print
        for (int i = 0; i < deviceCount; ++i) {
            ret += '{';
            ret += devices[i]->ToString();
            ret += '}';
            if (i < deviceCount - 1) ret += ",";
        }
        ret += ']';
        return ret;
    }

}