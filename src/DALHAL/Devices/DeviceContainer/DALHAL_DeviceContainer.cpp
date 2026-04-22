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

#include <DALHAL/Core/Manager/DALHAL_DeviceManager.h>
#include <DALHAL/Support/DALHAL_Logger.h>
#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>
#include <DALHAL/Core/JsonConfig/DALHAL_ArduinoJSON_ext.h>

#include <DALHAL/Devices/_Registry/DALHAL_DevicesRegistry.h>

#include "DALHAL_DeviceContainer_JSON_Schema.h"

#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Base.h>
#include <DALHAL/Core/JsonConfig/Types/Structures/DALHAL_JSON_Schema_ArrayOfRegistryItems.h>

namespace DALHAL {

    constexpr Registry::DefineBase DeviceContainer::RegistryDefine = {
        Create,
        &JsonSchema::DeviceContainer::Root,
        nullptr /* no events available */
    };
    
    DeviceContainer::~DeviceContainer() {
        if (devices) {
            for (int i = 0; i < deviceCount; ++i) {
                delete devices[i];
            }
            delete[] devices;
        }
    }
    // MAJOR TODO
    // fix DeviceManager PArseJson so that it can load into any devices array, and thus can be reused for any register
    // then fix DeviceContainer and other things loading devices so that they can use that instead
    DeviceContainer::DeviceContainer(DeviceCreateContext& context) : Device(context.deviceType) {
        const JsonVariant& jsonObj = *(context.jsonObjItem);
        uid = encodeUID(JsonSchema::GetValue(JsonSchema::CommonBase::uidFieldRequired, context).asConstChar());

        const JsonArray& jsonArray = JsonSchema::SchemaArrayOfRegistryItems::GetValidatedJsonArray(JsonSchema::DeviceContainer::itemsField, jsonObj);
        
        uint32_t deviceCountTmp = 0;
        int arraySize = jsonArray.size();

        // First pass: count valid entries
        for (int i=0;i<arraySize;i++) {
            if (Device::DisabledOrCommentItem(jsonArray[i]) == true) { continue; }
            deviceCountTmp++;
        }
        
        deviceCount = deviceCountTmp;
        if (deviceCount == 0) {
            devices = nullptr;
            GlobalLogger.Error(F("DeviceContainer JSON cfg does not contain any valid devices!\n" 
                                 "Hint: Check that all entries have 'type' and 'uid' fields, and match known types."));
            return;
        }

        // Allocate space for all devices
        devices = new Device*[deviceCount]();

        if (devices == nullptr) {
            deviceCount = 0;
            GlobalLogger.Error(F("Failed to allocate device array"));
            return;
        }

        // Second pass: actually create and store devices
        uint32_t index = 0;
        for (int i=0;i<arraySize;i++) {
            const JsonVariant& jsonItem = jsonArray[i];
            if (Device::DisabledOrCommentItem(jsonItem) == true) { continue; }
            devices[index++] = DeviceManager::CreateDeviceFromJSON(jsonItem);
        }
        std::string devCountStr = std::to_string(deviceCount);
        GlobalLogger.Info(F("Created sub devices: "), devCountStr.c_str());
    }

    Device* DeviceContainer::Create(DeviceCreateContext& context) {
        return new DeviceContainer(context);
    }

    String DeviceContainer::ToString() {
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

    void DeviceContainer::loop() {
        if (devices == nullptr || deviceCount == 0) return;
        for (int i=0;i<deviceCount;i++)
        {
            devices[i]->loop();
        }
    }

    void DeviceContainer::begin() {
        if (devices == nullptr || deviceCount == 0) return;
        for (int i=0;i<deviceCount;i++)
        {
            devices[i]->begin();
        }
    }
    
    DeviceFindResult DeviceContainer::findDevice(UIDPath& path, Device*& outDevice) {
        return Device::findInArray(devices, deviceCount, path, this, outDevice);
    }

}