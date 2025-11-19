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

#include "HAL_JSON_CachedDeviceRead.h"
#include "ScriptEngine/HAL_JSON_SCRIPT_ENGINE_Support.h"

namespace HAL_JSON {

    HALOperationResult CachedDeviceRead::ReadSimple(HALValue& val) {
        return handler(context, val);
    }

    struct FuncContext {
        Device* device;
        Device::ReadToHALValue_FuncType func;
    };

    struct BracketContext {
        Device* device;
        Device::BracketOpRead_FuncType bracketFunc;
        CachedDeviceRead* subscriptOperand; // owned
        ~BracketContext() {
            if (subscriptOperand)
                delete subscriptOperand; 
        }
    };

    HALOperationResult CachedDeviceRead::Handler_Invalid(void* ctx, HALValue& val) {
        return HALOperationResult::ExecutionFailed;
    }

    HALOperationResult CachedDeviceRead::Handler_Direct(void* ctx, HALValue& val) {
        HALValue* valPtr = static_cast<HALValue*>(ctx);
        val = *valPtr;
        return HALOperationResult::Success;
    }

    HALOperationResult CachedDeviceRead::Handler_Device(void* ctx, HALValue& val) {
        Device* device = static_cast<Device*>(ctx);
        return device->read(val);
    }

    HALOperationResult CachedDeviceRead::Handler_Func(void* ctx, HALValue& val) {
        FuncContext* c = static_cast<FuncContext*>(ctx);
        return c->func(c->device, val);
    }

    HALOperationResult CachedDeviceRead::Handler_Bracket(void* ctx, HALValue& val) {
        BracketContext* c = static_cast<BracketContext*>(ctx);
        HALValue subVal;
        HALOperationResult res = c->subscriptOperand->ReadSimple(subVal);
        if (res != HALOperationResult::Success) return res;

        if (c->bracketFunc)
            return c->bracketFunc(c->device, subVal, val);
        else
            return c->device->read(subVal, val);
    }
    CachedDeviceRead::~CachedDeviceRead() { if (deleter) deleter(context); }
    CachedDeviceRead::CachedDeviceRead() {
        context = nullptr;
        handler = nullptr;
        deleter = nullptr;
    }
    CachedDeviceRead::CachedDeviceRead(ZeroCopyString zcStrUidPathAndFuncName) 
    {
        context = nullptr;
        handler = nullptr;
        deleter = nullptr;
        Set(zcStrUidPathAndFuncName);
    }

    bool CachedDeviceRead::Set(ZeroCopyString zcStrUidPathAndFuncName) {
        if (deleter) deleter(context);

        context = nullptr;
        handler = nullptr;
        deleter = nullptr;

        const char* bracketPos = zcStrUidPathAndFuncName.FindChar('[');
        if (bracketPos) {
            // Bracket read
            ZeroCopyString inner(bracketPos + 1, zcStrUidPathAndFuncName.end - 1);
            zcStrUidPathAndFuncName.end = bracketPos;
            ZeroCopyString zcUid = zcStrUidPathAndFuncName.SplitOffHead('#');
            UIDPath uid(zcUid);
            Device* device = Manager::findDevice(uid);
            if (device == nullptr) {
                GlobalLogger.Error(F("CachedDeviceRead - bracket could not find source device: "), zcUid.ToString().c_str());
                handler = &CachedDeviceRead::Handler_Invalid;
                return false;
            }

            Device::BracketOpRead_FuncType bracketFunc = device->GetBracketOpRead_Function(zcStrUidPathAndFuncName);
            
            if (bracketFunc == nullptr && zcStrUidPathAndFuncName.NotEmpty()) {
                GlobalLogger.Error(F("CachedDeviceRead - bracket could not find source device function: "), zcStrUidPathAndFuncName.ToString().c_str());
                handler = &CachedDeviceRead::Handler_Invalid;
                return false;
            }

            CachedDeviceRead* subOperand = new CachedDeviceRead();
            if (subOperand->Set(inner) == false) {
                // this will logg the error separately
                //GlobalLogger.Error(F("CachedDeviceRead - bracket could not find source device function: "), zcStrUidPathAndFuncName.ToString().c_str());
                delete subOperand;
                handler = &CachedDeviceRead::Handler_Invalid;
                return false;
            }
            

            auto* ctx = new BracketContext{device, bracketFunc, subOperand};

            context = ctx;
            handler = &CachedDeviceRead::Handler_Bracket;
            deleter = ScriptEngine::DeleteAs<BracketContext>;
            return true;
        }

        // Funcname or plain read
        ZeroCopyString operandName = zcStrUidPathAndFuncName.SplitOffHead('#');
        ZeroCopyString& funcName = zcStrUidPathAndFuncName;
        UIDPath uid(operandName);
        Device* device = Manager::findDevice(uid);
        if (!device) {
            GlobalLogger.Error(F("CachedDeviceRead - could not find source device: "), operandName.ToString().c_str());
            handler = &CachedDeviceRead::Handler_Invalid;
            return false; 
        }

        // Func read
        Device::ReadToHALValue_FuncType readFunc = device->GetReadToHALValue_Function(funcName);
        if (readFunc == nullptr && funcName.NotEmpty()) {
            // this mean we requested to use a function name 
            // that did not exist thus the default is no op
            GlobalLogger.Error(F("CachedDeviceRead - could not find source device function: "), funcName.ToString().c_str());
            handler = &CachedDeviceRead::Handler_Invalid;
            return false;
        }
        if (readFunc) {
            auto* ctx = new FuncContext{device, readFunc};
            context = ctx;
            handler = &CachedDeviceRead::Handler_Func;
            deleter = ScriptEngine::DeleteAs<FuncContext>;
            return true;
        }

        // Direct access
        deleter = nullptr; // non owning
        HALValue* valPtr = device->GetValueDirectAccessPtr();
        if (valPtr) {
            context = valPtr;
            handler = &CachedDeviceRead::Handler_Direct;
            return true;
        }

        // Fallback device read
        context = device;
        handler = &CachedDeviceRead::Handler_Device;
        return true;
    }

}