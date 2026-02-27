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

#include "DALHAL_CachedDeviceAccess.h"

#include "../../Support/DALHAL_Logger.h"
#include "../Types/DALHAL_UID_Path.h"

#include "../Manager/DALHAL_DeviceManager.h"

namespace DALHAL {

    CachedDeviceAccess::~CachedDeviceAccess() {
        if (bracketAccessSubscriptOperand != nullptr)
            delete bracketAccessSubscriptOperand;
    }

    CachedDeviceAccess::CachedDeviceAccess() {
        readToHalValueFunc = nullptr;
        valueDirectAccessPtr = nullptr;
        execFunc = nullptr;
        bracketReadFunc = nullptr;
        bracketWriteFunc = nullptr;
        bracketAccessSubscriptOperand = nullptr;
    }
    bool CachedDeviceAccess::Set(const char* uidPathAndFuncName) { return Set(ZeroCopyString(uidPathAndFuncName)); }

    bool CachedDeviceAccess::Set(ZeroCopyString zcStrUidPathAndFuncName) {
        // allways set all to nullptr
        readToHalValueFunc = nullptr;
        valueDirectAccessPtr = nullptr;
        execFunc = nullptr;
        bracketReadFunc = nullptr;
        bracketWriteFunc = nullptr;
        bracketAccessSubscriptOperand = nullptr;

        // TODO take care of [] operator
        const char* bracketPos = zcStrUidPathAndFuncName.FindChar('[');
        if (bracketPos) {
            ZeroCopyString bracketVarOperand(bracketPos+1, zcStrUidPathAndFuncName.end-1);
            bracketAccessSubscriptOperand = new CachedDeviceAccess();
            if (bracketAccessSubscriptOperand->Set(bracketVarOperand) == false) {
                delete bracketAccessSubscriptOperand;
                bracketAccessSubscriptOperand = nullptr;
                return false;
            }
            zcStrUidPathAndFuncName.end = bracketPos;
        }

        ZeroCopyString zcStrUidPath = zcStrUidPathAndFuncName.SplitOffHead('#');
        ZeroCopyString& zcStrFuncName = zcStrUidPathAndFuncName;

        UIDPath uidPath(zcStrUidPath);
        DeviceFindResult devFindRes = DeviceManager::findDevice(uidPath, device);

        if (devFindRes != DeviceFindResult::Success) {
            
            std::string uidStr = uidPath.ToString();
            printf("@CachedDeviceAccess const - %s:>>%s<<\n", DeviceFindResultToString(devFindRes), uidStr.c_str());
            return false;
        }
        //if (zcStrFuncName.Length() != 0)
        //    printf("create cached device access: %s#%s\n", uidPath.ToString().c_str(), zcStrFuncName.ToString().c_str());
        //else
        //    printf("create cached device access: %s\n", uidPath.ToString().c_str());
        readToHalValueFunc = device->GetReadToHALValue_Function(zcStrFuncName);
        writeFromHalValueFunc = device->GetWriteFromHALValue_Function(zcStrFuncName);
        execFunc = device->GetExec_Function(zcStrFuncName);
        bracketReadFunc = device->GetBracketOpRead_Function(zcStrFuncName);
        bracketWriteFunc = device->GetBracketOpWrite_Function(zcStrFuncName);
        
        valueDirectAccessPtr = device->GetValueDirectAccessPtr();
        return true;
    }

    HALOperationResult CachedDeviceAccess::Exec() {
        if (execFunc != nullptr) {
            //printf("\CDA Exec - no params\n");
            return execFunc(device);
        }
        return HALOperationResult::UnsupportedOperation;
    }

    HALOperationResult CachedDeviceAccess::WriteSimple(const HALValue& val) {
        if (bracketAccessSubscriptOperand != nullptr) {
            HALValue subscriptValue;
            HALOperationResult readRes = bracketAccessSubscriptOperand->ReadSimple(subscriptValue);
            if (readRes != HALOperationResult::Success)
                return readRes;
            if (bracketWriteFunc == nullptr)
                return device->write(subscriptValue, val);
            else
                return bracketWriteFunc(device, subscriptValue, val);
        }
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
        if (bracketAccessSubscriptOperand != nullptr) {
            HALValue subscriptValue;
            HALOperationResult readRes = bracketAccessSubscriptOperand->ReadSimple(subscriptValue);
            if (readRes != HALOperationResult::Success)
                return readRes;
            if (bracketReadFunc == nullptr)
                return device->read(subscriptValue, val);
            else
                return bracketReadFunc(device, subscriptValue, val);
        }
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