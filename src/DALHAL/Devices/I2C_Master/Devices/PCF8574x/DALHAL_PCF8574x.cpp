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

#include "DALHAL_PCF8574x.h"

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>

#include <DALHAL/Support/DALHAL_Logger.h>
#include <DALHAL/Core/JsonConfig/DALHAL_ArduinoJSON_ext.h>
#include <DALHAL/Support/ConvertHelper.h>
#include <DALHAL/Core/Types/DALHAL_Registry.h>

#include "DALHAL_PCF8574x_JSON_Schema.h"

namespace DALHAL {

    __attribute__((used, externally_visible))
    constexpr I2C_RegistryDefine PCF8574x::RegistryDefine = {
        Create,
        &JsonSchema::PCF8574x::Root,
        DALHAL_REACTIVE_EVENT_TABLE(I2C_DEVICE_PCF8574X),
        HasAddress
    };
    //volatile const void* keep_PCF8574x = &DALHAL::PCF8574x::RegistryDefine;

    bool PCF8574x::HasAddress(uint8_t addr) {
        return (addr >= 0x20 && addr <= 0x27) || // PCF8574
               (addr >= 0x38 && addr <= 0x3f);   // PCF8574A
    }
    
    PCF8574x::PCF8574x(I2C_Master_CreateFunctionContext& context) : PCF8574x_DeviceBase(context.deviceType), wire(context.wire) {
        JsonSchema::PCF8574x::Extractors::Apply(context, this);
    }

    Device* PCF8574x::Create(DeviceCreateContext& context) {
        return new PCF8574x(static_cast<I2C_Master_CreateFunctionContext&>(context));
    }

    HALOperationResult PCF8574x::read(HALValue& val) {
        uint8_t received = wire.requestFrom(addr, (uint8_t)1);
        if (received == 0) return HALOperationResult::ExecutionFailed;
        val = (uint32_t)wire.read();
#if HAS_REACTIVE_READ(I2C_DEVICE_PCF8574X)
        triggerRead();
#endif
        return HALOperationResult::Success;
    }
    HALOperationResult PCF8574x::write(const HALValue& val) {
        if (val.getType() == HALValue::Type::TEST) return HALOperationResult::Success; // test write to check feature
        if (val.isNaN()) return HALOperationResult::WriteValueNaN;
        wire.beginTransmission(addr);
        wire.write(val.asUInt());
        uint8_t res = wire.endTransmission(true);
        if (res != 0) {
            // todo maybe log to global logger
            return HALOperationResult::ExecutionFailed;
        }
#if HAS_REACTIVE_WRITE(I2C_DEVICE_PCF8574X)
        triggerWrite();
#endif
        return HALOperationResult::Success;
    }

    String PCF8574x::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += this->Type;
        ret += "\",";
        ret += "\"addr\":\"0x";
        ret += Convert::toHex(addr).c_str();
        ret += "\"";
        return ret;
    }

}