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

#include "DALHAL_ScriptVariableWriteOnlyTest.h"

#include <DALHAL/Support/DALHAL_Logger.h>
#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>
#include <DALHAL/Core/JsonConfig/DALHAL_ArduinoJSON_ext.h>

#include "DALHAL_ScriptVariableWriteOnlyTest_JSON_Schema.h"

namespace DALHAL {
    constexpr Registry::DefineBase ScriptVariableWriteOnlyTest::RegistryDefine = {
        Create,
        &JsonSchema::ScriptVariableWriteOnlyTest,
        DALHAL_REACTIVE_EVENT_TABLE(SCRIPT_WRITEVAR)
    };
    
    ScriptVariableWriteOnlyTest::ScriptVariableWriteOnlyTest(DeviceCreateContext& context) : ScriptVariableWriteOnlyTest_DeviceBase(context.deviceType) {
        const JsonVariant& jsonObj = *(context.jsonObjItem);
        uid = encodeUID(GetAsConstChar(jsonObj,DALHAL_KEYNAME_UID));
        value = GetAsUINT32(jsonObj, "val",0);
    }

    Device* ScriptVariableWriteOnlyTest::Create(DeviceCreateContext& context) {
        return new ScriptVariableWriteOnlyTest(context);
    }

    HALOperationResult ScriptVariableWriteOnlyTest::write(const HALValue& val) {
        if (val.getType() == HALValue::Type::TEST) return HALOperationResult::Success; // test write to check feature
        if (val.isNaN()) return HALOperationResult::WriteValueNaN;
        value = val;
#if HAS_REACTIVE_WRITE(SCRIPT_WRITEVAR)
        triggerWrite();
#endif
        return HALOperationResult::Success;
    }

    String ScriptVariableWriteOnlyTest::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += this->Type;
        ret += "\",";
        ret += DeviceConstStrings::value;//StartWithComma;
        if (value.getType() == HALValue::Type::FLOAT)
            ret += std::to_string(value.asFloat()).c_str();
        else if (value.getType() == HALValue::Type::UINT)
            ret += std::to_string(value.asUInt()).c_str();
        else if (value.getType() == HALValue::Type::INT)
            ret += std::to_string(value.asInt()).c_str();
        else
            ret += "\"not set\"";
        return ret;
    }

}