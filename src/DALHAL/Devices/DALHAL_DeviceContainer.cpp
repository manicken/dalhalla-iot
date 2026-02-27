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

#include "DALHAL_DeviceContainer.h"
#include "../Core/Manager/DALHAL_DeviceManager.h"
#include "../Support/DALHAL_Logger.h"
#include "../Core/Device/DALHAL_JSON_Config_Defines.h"
#include "../Support/DALHAL_ArduinoJSON_ext.h"

namespace DALHAL {
    
    DeviceContainer::~DeviceContainer() {
        if (devices) {
            for (int i = 0; i < deviceCount; ++i) {
                delete devices[i];
            }
            delete[] devices;
        }
    }

    DeviceContainer::DeviceContainer(const JsonVariant &jsonObj, const char* type) : Device(type) {
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
            bool valid = DeviceManager::VerifyDeviceJson(jsonItem);
            validDevices[i] = valid;
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
            JsonVariant jsonItem = jsonArray[i];
            if (validDevices[i] == false) continue;
            devices[index++] = DeviceManager::CreateDeviceFromJSON(jsonItem);
        }
        std::string devCountStr = std::to_string(deviceCount);
        GlobalLogger.Info(F("Created sub devices: "), devCountStr.c_str());

        delete[] validDevices; // free memory
    }

    bool DeviceContainer::VerifyJSON(const JsonVariant &jsonObj) {
        if (jsonObj.containsKey(DALHAL_KEYNAME_ITEMS) == false) {
            GlobalLogger.Error(DALHAL_ERR_MISSING_KEY(DALHAL_KEYNAME_ITEMS));
            SET_ERR_LOC(DALHAL_ERROR_SOURCE_I2C_VERIFY_JSON);
            return false;
        }
        if (jsonObj[DALHAL_KEYNAME_ITEMS].is<JsonArray>() == false) {
            GlobalLogger.Error(DALHAL_ERR_VALUE_TYPE(DALHAL_KEYNAME_ITEMS " not array"));
            SET_ERR_LOC(DALHAL_ERROR_SOURCE_I2C_VERIFY_JSON);
            return false;
        }
        const JsonArray items = jsonObj[DALHAL_KEYNAME_ITEMS].as<JsonArray>();
        if (items.size() == 0) {
            GlobalLogger.Error(DALHAL_ERR_ITEMS_EMPTY());
            SET_ERR_LOC(DALHAL_ERROR_SOURCE_I2C_VERIFY_JSON);
            return false;
        }
        int arraySize = items.size();
        for (int i=0;i<arraySize;i++) {
            const JsonVariant& jsonItem = items[i];
            if (IsConstChar(jsonItem) == true) { continue; } // comment item
            if (Device::DisabledInJson(jsonItem) == true) { continue; } // disabled
            bool valid = DeviceManager::VerifyDeviceJson(jsonItem);
            if (valid == false) DALHAL_VALIDATE_IN_LOOP_FAIL_OPERATION; // could either be continue; or return false depending if strict mode is on/off
        }
        return true;
    }

    Device* DeviceContainer::Create(const JsonVariant &jsonObj, const char* type) {
        return new DeviceContainer(jsonObj, type);
    }

    String DeviceContainer::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += type;
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