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

#include "DALHAL_TX433_Unit.h"

#include <DALHAL/Core/JsonConfig/DALHAL_ArduinoJSON_ext.h>
#include <DALHAL/Support/DALHAL_Logger.h>
#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>

#include <DALHAL/Support/ConvertHelper.h>

#include <DALHAL/Core/Types/DALHAL_Registry.h>

#include "DALHAL_TX433_Unit_TypeLC_JSON_Schema.h"
#include "DALHAL_TX433_Unit_TypeSFC_JSON_Schema.h"
#include "DALHAL_TX433_Unit_TypeAFC_JSON_Schema.h"

namespace DALHAL {
    constexpr Registry::DefineBase TX433_Unit::LCTypeRegistryDefine = {
        Create,
        &JsonSchema::TX433_Unit_TypeLC,
        DALHAL_REACTIVE_EVENT_TABLE(TX433_UNIT)
    };
    
    constexpr Registry::DefineBase TX433_Unit::SFCTypeRegistryDefine = {
        Create,
        &JsonSchema::TX433_Unit_TypeSFC,
        DALHAL_REACTIVE_EVENT_TABLE(TX433_UNIT)
    };
    
    constexpr Registry::DefineBase TX433_Unit::AFCTypeRegistryDefine = {
        Create,
        &JsonSchema::TX433_Unit_TypeAFC,
        DALHAL_REACTIVE_EVENT_TABLE(TX433_UNIT)
    };

    Device* TX433_Unit::Create(DeviceCreateContext& context) {
        return new TX433_Unit(static_cast<TX433_Unit_CreateFunctionContext&>(context));
    }
    

   /* bool TX433_Unit::VerifyFC_JSON(const JsonVariant &jsonObj) {
        if (!ValidateJsonStringField(jsonObj,"ch")) { SET_ERR_LOC(DALHAL_ERROR_SOURCE_TX433_UNIT_VERIFY_FC_JSON); return false; }
        if (!ValidateJsonStringField(jsonObj,"btn")) { SET_ERR_LOC(DALHAL_ERROR_SOURCE_TX433_UNIT_VERIFY_FC_JSON); return false; }
        //if (!ValidateJsonStringField(jsonObj,"state")) return false;
        return true;
    }*/
    
    TX433_Unit::TX433_Unit(TX433_Unit_CreateFunctionContext& context) : TX433unit_DeviceBase(context.deviceType), pin(context.pin) {
        const JsonVariant& jsonObj = *(context.jsonObjItem);
        const char* uidStr = jsonObj[DALHAL_KEYNAME_UID].as<const char*>();
        uid = encodeUID(uidStr);
        const char* modelStr = GetAsConstChar(jsonObj, DALHAL_KEYNAME_TX433_MODEL);
        
        if (strcasecmp(modelStr, "lc") == 0) {
            staticData = RF433::Get433_LC_Data(jsonObj);
            model = TX433_MODEL::LearningCode;
        }
        else if (strcasecmp(modelStr, "sfc") == 0) {
            staticData = RF433::Get433_SFC_Data(jsonObj);
            model = TX433_MODEL::FixedCode;
        }
        else if (strcasecmp(modelStr, "afc") == 0) {
            staticData = RF433::Get433_AFC_Data(jsonObj);
            model = TX433_MODEL::FixedCode;
        }
        //else this will never happen if VerifyJSON is used beforehand

        fixedState = (jsonObj.containsKey(DALHAL_KEYNAME_TX433_STATE) && IsUINT32(jsonObj,DALHAL_KEYNAME_TX433_STATE));
    }

    HALOperationResult TX433_Unit::write(const HALValue &val) {
        if (val.getType() == HALValue::Type::TEST) return HALOperationResult::Success; // test write to check feature
        if (val.isNaN()) return HALOperationResult::WriteValueNaN;
        RF433::init(pin); // ensure that the correct pin is used and that it's set to a output
        if (model == TX433_MODEL::FixedCode) {
            if (fixedState == false) {
                RF433::SendTo433_FC(staticData, val.asUInt());
            } else {
                RF433::SendTo433_FC(staticData);
            }
        }
        else if (model == TX433_MODEL::LearningCode) {
            if (fixedState == false) {
                RF433::SendTo433_LC(staticData, val.asUInt());
            } else {
                RF433::SendTo433_LC(staticData);
            }
        } else {
            return HALOperationResult::ExecutionFailed; // this will never happend
        }
#if HAS_REACTIVE_WRITE(TX433_UNIT)
        triggerWrite();
#endif
        return HALOperationResult::Success;
    }

    String TX433_Unit::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        
        ret += "\"model\":\"";
        if (model == TX433_MODEL::LearningCode)
            ret += "LC";
        else if (model == TX433_MODEL::FixedCode)
            ret += "FC";
        
        ret += "\",";
        ret += "\"data\":\"";
        ret += Convert::toHex(staticData).c_str();
        ret += "\"";
        return ret;
    }

}