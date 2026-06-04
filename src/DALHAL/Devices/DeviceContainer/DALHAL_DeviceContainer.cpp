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

#include "DALHAL_DeviceContainer.h"

#include <DALHAL/Support/DALHAL_Logger.h>

#include "DALHAL_DeviceContainer_JSON_Schema.h"

namespace DALHAL {

    __attribute__((used, externally_visible))
    constexpr Registry::DefineBase DeviceContainer::RegistryDefine = {
        Create,
        &JsonSchema::DeviceContainer::Root
    };
    //volatile const void* keep_DeviceContainer = &DALHAL::DeviceContainer::RegistryDefine;
    
    DeviceContainer::~DeviceContainer() {
        if (devices) {
            for (int i = 0; i < deviceCount; ++i) {
                delete devices[i];
            }
            delete[] devices;
        }
    }

    Device* DeviceContainer::Create(DeviceCreateContext& context) {
        return new DeviceContainer(context);
    }

    DeviceContainer::DeviceContainer(DeviceCreateContext& context) : Device(context.deviceType) {
        JsonSchema::DeviceContainer::Extractors::Apply(context, this);
    }

    void DeviceContainer::begin() {
        if (devices == nullptr || deviceCount == 0) return;
        for (int i=0;i<deviceCount;i++)
        {
            devices[i]->begin();
        }
    }

    void DeviceContainer::loop() {
        if (devices == nullptr || deviceCount == 0) return;
        for (int i=0;i<deviceCount;i++)
        {
            devices[i]->loop();
        }
    }

    DeviceFindResult DeviceContainer::findDevice(UIDPath& path, Device*& outDevice) {
        return Device::findInArray(devices, deviceCount, path, this, outDevice);
    }

    void DeviceContainer::PrintTo(StringBuilderStreamer& sbs) {
        Device::PrintTo(sbs);

        sbs.write_json_value_separator();
        sbs.write_jsonKey(F("items"));
        sbs.write_json_array_begin();
        
        for (int i = 0; i < deviceCount; ++i) {
            if (i > 0) { sbs.write_json_value_separator(); }
            sbs.write_json_object_begin();
            devices[i]->PrintTo(sbs);
            sbs.write_json_object_end();
        }
        sbs.write_json_array_end();
    }

}