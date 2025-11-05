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

#include "HAL_JSON_ScriptArray.h"

namespace HAL_JSON {
    
    ScriptArray::ScriptArray(const JsonVariant &jsonObj, const char* type) : Device(UIDPathMaxLength::One,type) {
        uid = encodeUID(GetAsConstChar(jsonObj,HAL_JSON_KEYNAME_UID));

    }

    bool ScriptArray::VerifyJSON(const JsonVariant &jsonObj) {

        return true;
    }

    Device* ScriptArray::Create(const JsonVariant &jsonObj, const char* type) {
        return new ScriptArray(jsonObj, type);
    }

    String ScriptArray::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += type;
        ret += "\"";
        return ret;
    }
    // init array here
    void ScriptArray::begin() {

    }

    HALOperationResult ScriptArray::read(const HALValue& bracketSubscriptVal, HALValue& val) {
        int index = bracketSubscriptVal.asInt();
        if (index < 0 || index >= valueCount) {
            printf("\nScriptArray::read BracketOpSubscriptOutOffRange:%d\n", index);
            return HALOperationResult::BracketOpSubscriptOutOffRange;
        }
        val = values[index];
        return HALOperationResult::Success;
    }

    HALOperationResult ScriptArray::write(const HALValue& bracketSubscriptVal, const HALValue& val) {
        int index = bracketSubscriptVal.asInt();
        if (index < 0 || index >= valueCount) {
            printf("\nScriptArray::write BracketOpSubscriptOutOffRange:%d\n", index);
            return HALOperationResult::BracketOpSubscriptOutOffRange;
        }
        values[index] = val;
        return HALOperationResult::Success;
    }
/*
    HALOperationResult ScriptArray::BracketRead_Func(Device* device, const HALValue& bracketSubscriptVal, HALValue& val) {
        return static_cast<ScriptArray*>(device)->read(bracketSubscriptVal, val);
    }
    HALOperationResult ScriptArray::BracketWrite_Func(Device* device, const HALValue& bracketSubscriptVal, const HALValue& val) {
        return static_cast<ScriptArray*>(device)->write(bracketSubscriptVal, val);
    }
    
    Device::BracketOpRead_FuncType ScriptArray::GetBracketOpRead_Function(ZeroCopyString& zcFuncName) {
        return ScriptArray::BracketRead_Func; // functionname not used at the moment
    }

    Device::BracketOpWrite_FuncType ScriptArray::GetBracketOpWrite_Function(ZeroCopyString& zcFuncName) {
        return ScriptArray::BracketWrite_Func; // functionname not used at the moment
    }
*/

}