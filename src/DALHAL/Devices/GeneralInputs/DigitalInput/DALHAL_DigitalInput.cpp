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

#include "DALHAL_DigitalInput.h"

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>
#include <DALHAL/Core/Manager/DALHAL_GPIO_Manager.h>
#include <DALHAL/Core/JsonConfig/DALHAL_ArduinoJSON_ext.h>
#include <DALHAL/Support/DALHAL_Logger.h>


namespace DALHAL {
    
    Device* DigitalInput::Create(DeviceCreateContext& context) {
        return new DigitalInput(context);
    }

    bool DigitalInput::VerifyJSON(const JsonVariant &jsonObj) {
        return GPIO_manager::ValidateJsonAndCheckIfPinAvailableAndReserve(jsonObj, static_cast<uint8_t>(GPIO_manager::PinFunc::IN));
    }

    DigitalInput::DigitalInput(DeviceCreateContext& context) : DigitalInput_DeviceBase(context.deviceType) {
        const JsonVariant& jsonObj = *(context.jsonObjItem);
        pin = GetAsUINT32(jsonObj, DALHAL_KEYNAME_PIN);// jsonObj[DALHAL_KEYNAME_PIN];// | 0;//.as<uint8_t>();
        uid = encodeUID(GetAsConstChar(jsonObj, DALHAL_KEYNAME_UID));
        //pin = jsonObj[DALHAL_KEYNAME_PIN];//.as<uint8_t>();
        GPIO_manager::ReservePin(pin);
        
        //const char* uidStr = jsonObj[DALHAL_KEYNAME_UID];//.as<const char*>();
        //uid = encodeUID(uidStr);

        pinMode(pin, INPUT); // input
    }

    HALOperationResult DigitalInput::read(HALValue &val) {
        //val.set((uint32_t)digitalRead(pin));
        val = (uint32_t)digitalRead(pin);
#if HAS_REACTIVE_READ(DIGITAL_INPUT)
        triggerRead();
#endif
        return HALOperationResult::Success;
    }

    String DigitalInput::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += this->Type;
        ret += DeviceConstStrings::pin;
        ret += std::to_string(pin).c_str();
        ret += ",";
        ret += DeviceConstStrings::value;//StartWithComma;
        ret += std::to_string(digitalRead(pin)).c_str();
        return ret;
    }
	
}