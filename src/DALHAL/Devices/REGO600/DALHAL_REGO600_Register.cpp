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

#include "DALHAL_REGO600_Register.h"
#include <DALHAL/Drivers/REGO600.h>

#include <DALHAL/Core/JsonConfig/DALHAL_ArduinoJSON_ext.h>
#include <DALHAL/Support/DALHAL_Logger.h>
#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>
#include <DALHAL/Core/Types/DALHAL_Registry.h>

namespace DALHAL {
    
    REGO600_Register::REGO600_Register(DeviceCreateContext& context) : REGO600register_DeviceBase(context.deviceType) {
        const JsonVariant& jsonObj = *(context.jsonObjItem);
        const char* uidStr = jsonObj[DALHAL_KEYNAME_UID].as<const char*>();
        uid = encodeUID(uidStr);

#if HAS_REACTIVE_VALUE_CHANGE(SCRIPT_VARIABLE)
        value.setCallbacks(this, GenericValueCallback<REGO600register_DeviceBase>, nullptr);
#endif
    }
/* keep this here as in the future we may want to use Drivers::REGO600::SystemRegisterTableLockup(regName);
    if (ValidateJsonStringField(jsonObj, "regname") == false) { SET_ERR_LOC(DALHAL_ERROR_SOURCE_REGO600_REG_VERIFY_JSON); anyError = true; }
        const char* regName = GetAsConstChar(jsonObj, "regname");
        const Drivers::REGO600::RegoLookupEntry* entry = Drivers::REGO600::SystemRegisterTableLockup(regName);
        if (entry == nullptr) {
            GlobalLogger.Error(F("regname not found"), regName);
            SET_ERR_LOC(DALHAL_ERROR_SOURCE_REGO600_REG_VERIFY_JSON);
            anyError = true;
        }
        
        return anyError == false;
    }
*/

    HALOperationResult REGO600_Register::read(HALValue& val) {
        val = value;
        return HALOperationResult::Success;
    }

    String REGO600_Register::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += '"';
        return ret;
    }

}