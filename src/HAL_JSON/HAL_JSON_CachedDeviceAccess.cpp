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

#include "HAL_JSON_CachedDeviceAccess.h"

namespace HAL_JSON {

    CachedDeviceAccess::CachedDeviceAccess(ZeroCopyString& uidPath, ZeroCopyString& funcName) : path(uidPath) {
        device = Manager::findDevice(path);
        currentVersion = Manager::ReloadVersionPtr();
        cachedVersion = *currentVersion;
        // allways set all to nullptr
        readToHalValueFunc = nullptr;
        valueDirectAccessPtr = nullptr;
        execFunc = nullptr;

        if (device == nullptr) {
            
            std::string uidStr = uidPath.ToString();
            printf("@CachedDeviceAccess const - device not found:>>%s<<\n", uidStr.c_str());
            return;
        }
        if (funcName != nullptr && device != nullptr) {
            printf("create cached device access: %s\n", funcName.ToString().c_str());
            readToHalValueFunc = device->GetReadToHALValue_Function(funcName);
            writeFromHalValueFunc = device->GetWriteFromHALValue_Function(funcName);
            execFunc = device->GetExec_Function(funcName);
        }
        
        valueDirectAccessPtr = device->GetValueDirectAccessPtr();
        
    }

    Device* CachedDeviceAccess::GetDevice() {
        if (cachedVersion != *currentVersion) {
            device = Manager::findDevice(path);
            cachedVersion = *currentVersion; 
        }
        return device;
    }
    HALOperationResult CachedDeviceAccess::Exec() {
        if (execFunc != nullptr) {
            Device* device = GetDevice();
            if (device == nullptr) return HALOperationResult::DeviceNotFound;
            return execFunc(device);
        }
        return HALOperationResult::UnsupportedOperation;
    }

    HALOperationResult CachedDeviceAccess::WriteSimple(const HALValue& val) {
        if (writeFromHalValueFunc != nullptr) {
            Device* device = GetDevice();
            if (device == nullptr) return HALOperationResult::DeviceNotFound;
            HALValue valToWrite = val;
            //printf("\nWriteSimple - writeFromHalValueFunc\n");
            return writeFromHalValueFunc(device, valToWrite);
        }
        if (valueDirectAccessPtr != nullptr) {
            *valueDirectAccessPtr = val;
            return HALOperationResult::Success;
        }
        //printf("\nWriteSimple - device->write(val)\n");
        Device* device = GetDevice();
        if (device == nullptr) return HALOperationResult::DeviceNotFound;
        return device->write(val);
    }

    HALOperationResult CachedDeviceAccess::ReadSimple(HALValue& val) {
        if (readToHalValueFunc != nullptr) {
            Device* device = GetDevice();
            if (device == nullptr) return HALOperationResult::DeviceNotFound;
            return readToHalValueFunc(device, val);
        }
        if (valueDirectAccessPtr != nullptr) {
            val = *valueDirectAccessPtr;
            return HALOperationResult::Success;
        }
        Device* device = GetDevice();
        if (device == nullptr) return HALOperationResult::DeviceNotFound;
        return device->read(val);
    }
}