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

    CachedDeviceAccess::CachedDeviceAccess(const char* uidPathAndFuncName) {
        ZeroCopyString funcName(uidPathAndFuncName);
        ZeroCopyString zcStrUidPath = funcName.SplitOffHead('#');
        Init(zcStrUidPath, funcName);
    }

    CachedDeviceAccess::CachedDeviceAccess(ZeroCopyString zcStrUidPathAndFuncName) {
        //ZeroCopyString funcName = zcStrUidPathAndFuncName; // copy
        ZeroCopyString zcStrUidPath = zcStrUidPathAndFuncName.SplitOffHead('#');
        Init(zcStrUidPath, zcStrUidPathAndFuncName);
    }

    CachedDeviceAccess::CachedDeviceAccess(ZeroCopyString& zcStrUidPath, ZeroCopyString& zcStrFuncName) {
        Init(zcStrUidPath, zcStrFuncName);
    }

    void CachedDeviceAccess::Init(ZeroCopyString& zcStrUidPath, ZeroCopyString& zcStrFuncName) {
        UIDPath uidPath(zcStrUidPath);
        device = Manager::findDevice(uidPath);

        // allways set all to nullptr
        readToHalValueFunc = nullptr;
        valueDirectAccessPtr = nullptr;
        execFunc = nullptr;
        readArrayFunc = nullptr;
        writeArrayFunc = nullptr;

        if (device == nullptr) {
            
            std::string uidStr = uidPath.ToString();
            printf("@CachedDeviceAccess const - device not found:>>%s<<\n", uidStr.c_str());
            return;
        }
        if (zcStrFuncName != nullptr) {
            //printf("create cached device access: %s#%s\n", uidPath.ToString().c_str(), zcStrFuncName.ToString().c_str());
            readToHalValueFunc = device->GetReadToHALValue_Function(zcStrFuncName);
            writeFromHalValueFunc = device->GetWriteFromHALValue_Function(zcStrFuncName);
            execFunc = device->GetExec_Function(zcStrFuncName);
        }
        
        valueDirectAccessPtr = device->GetValueDirectAccessPtr();
    }

    HALOperationResult CachedDeviceAccess::Exec() {
        if (execFunc != nullptr) {
            //printf("\CDA Exec - no params\n");
            return execFunc(device);
        }
        return HALOperationResult::UnsupportedOperation;
    }

    HALOperationResult CachedDeviceAccess::WriteSimple(const HALValue& val) {
        if (writeFromHalValueFunc != nullptr) {
            //printf("\nCDA WriteSimple - writeFromHalValueFunc\n");
            return writeFromHalValueFunc(device, val);
        }
        if (valueDirectAccessPtr != nullptr) {
            //printf("\nCDA ReadSimple - valueDirectAccessPtr\n");
            *valueDirectAccessPtr = val;
            return HALOperationResult::Success;
        }
        //printf("\nCDA WriteSimple - device->write(val)\n");
        return device->write(val);
    }

    HALOperationResult CachedDeviceAccess::ReadSimple(HALValue& val) {
        if (readToHalValueFunc != nullptr) {
            //printf("\nCDA ReadSimple - readToHalValueFunc\n");
            return readToHalValueFunc(device, val);
        }
        if (valueDirectAccessPtr != nullptr) {
            //printf("\nCDA ReadSimple - valueDirectAccessPtr\n");
            val = *valueDirectAccessPtr;
            return HALOperationResult::Success;
        }
        //printf("\nCDA ReadSimple - device->read(val)\n");
        return device->read(val);
    }
}