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

#include <DALHAL/Devices/HomeAssistant/DALHAL_HomeAssistant.h>
#include <DALHAL/Devices/HomeAssistant/DALHAL_HA_DeviceTypeReg.h>

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>
#include <DALHAL/Core/JsonConfig/DALHAL_ArduinoJSON_ext.h>
#include <DALHAL/Support/DALHAL_Logger.h>

#include "DALHAL_HA_DeviceContainer_JSON_Schema.h"

namespace DALHAL {
    constexpr Registry::DefineBase HA_DeviceContainer::RegistryDefine = {
        Create,
        &JsonSchema::HA_DeviceContainer,
    };
    
    HA_DeviceContainer::~HA_DeviceContainer() {
        if (devices) {
            for (int i = 0; i < deviceCount; ++i) {
                delete devices[i];
            }
            delete[] devices;
        }
    }

    HA_DeviceContainer::HA_DeviceContainer(HA_CreateFunctionContext& context) : Device(context.deviceType) {
        const JsonVariant& jsonObj = *(context.jsonObjItem);
        uid = encodeUID(GetAsConstChar(jsonObj,DALHAL_KEYNAME_UID));
        const JsonArray& jsonArray = jsonObj[DALHAL_KEYNAME_ITEMS].as<JsonArray>();
        
        uint32_t deviceCountTmp = 0;
        int arraySize = jsonArray.size();
        bool* validDevices = new bool[arraySize]; // dont' forget the delete[] call at end of function

        // First pass: count valid entries
        for (int i=0;i<arraySize;i++) {
            JsonVariant jsonItem = jsonArray[i];
            if (IsConstChar(jsonItem) == true) { validDevices[i] = false;  continue; } // comment item
            if (Device::DisabledInJson(jsonItem) == true) { validDevices[i] = false;  continue; } // disabled
            const char* type_cStr = GetAsConstChar(jsonItem, DALHAL_KEYNAME_TYPE);
            /*const Registry::Item& regItem = */Registry::GetItem(HA_DeviceRegistry, type_cStr);
            
            validDevices[i] = true; // allways valid in strict mode
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
            if (validDevices[i] == false) continue;
            const char* type_cStr = GetAsConstChar(jsonItem, DALHAL_KEYNAME_TYPE);
            const Registry::Item& regItem = Registry::GetItem(HA_DeviceRegistry, type_cStr);
            context.jsonObjItem = &jsonItem;
            context.deviceType = regItem.typeName;
            devices[index++] = regItem.def->Create_Function(context);
        }
        std::string devCountStr = std::to_string(deviceCount);
        GlobalLogger.Info(F("Created sub devices: "), devCountStr.c_str());

        delete[] validDevices; // free memory
    }

    Device* HA_DeviceContainer::Create(DeviceCreateContext& context) {
        return new HA_DeviceContainer(static_cast<HA_CreateFunctionContext&>(context));
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

    void HA_DeviceContainer::loop() {
        if (devices == nullptr || deviceCount == 0) return;
        for (int i=0;i<deviceCount;i++)
        {
            devices[i]->loop();
        }
    }

    void HA_DeviceContainer::begin() {
        if (devices == nullptr || deviceCount == 0) return;
        for (int i=0;i<deviceCount;i++)
        {
            devices[i]->begin();
        }
    }
    
    DeviceFindResult HA_DeviceContainer::findDevice(UIDPath& path, Device*& outDevice) {
        return Device::findInArray(devices, deviceCount, path, this, outDevice);
    }

}