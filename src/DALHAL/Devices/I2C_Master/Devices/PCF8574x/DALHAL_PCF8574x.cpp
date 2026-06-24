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
        &PCF8574x::FunctionTable,
        HasAddress
    };
    
    /* override */
    const Registry::DefineBase* PCF8574x::GetRegistryDefine() {
        return &RegistryDefine;
    }

    bool PCF8574x::HasAddress(uint8_t addr) {
        return (addr >= 0x20 && addr <= 0x27) || // PCF8574
               (addr >= 0x38 && addr <= 0x3f);   // PCF8574A
    }

    constexpr FunctionEntry<FunctionTypes::ReadToHALValue> PCF8574x::readValueFunctions[] = {
        DALHAL_PRIMARY_FUNCTION_ENTRY(HALValue_primary_read, "read from the device")
    };

    constexpr FunctionEntry<FunctionTypes::WriteHALValue> PCF8574x::writeValueFunctions[] = {
        DALHAL_PRIMARY_FUNCTION_ENTRY_WITH_VAL_TYPE(HALValue_primary_write, "write the value to the device", FunctionValueType::_UInt_),
    };

    constexpr DeviceFunctionTable PCF8574x::FunctionTable = {
        EmptyFunctionTable<FunctionTypes::Exec>,

        DALHAL_FUNCTION_TABLE_ENTRY(readValueFunctions),
        DALHAL_FUNCTION_TABLE_ENTRY(writeValueFunctions),

        EmptyFunctionTable<FunctionTypes::BracketOpRead>,
        EmptyFunctionTable<FunctionTypes::BracketOpWrite>,

        EmptyFunctionTable<FunctionTypes::ReadString>,
        EmptyFunctionTable<FunctionTypes::WriteString>,
    };
    
    PCF8574x::PCF8574x(I2C_Master_CreateFunctionContext& context) : PCF8574x_DeviceBase(context.deviceType), wire(context.wire) {
        JsonSchema::PCF8574x::Extractors::Apply(context, this);
    }

    Device* PCF8574x::Create(DeviceCreateContext& context) {
        return new PCF8574x(static_cast<I2C_Master_CreateFunctionContext&>(context));
    }

    void PCF8574x::loop() {
#if HAS_REACTIVE_CUSTOM(I2C_DEVICE_PCF8574X)
        if (intPin < 0) { return; }
        if (digitalRead(intPin) == LOW && intPinPrevState == 1) {
            intPinPrevState = 0;
            triggerInterrupt();
        } else {
            intPinPrevState = 1;
        }
#endif
    }

    HALOperationResult PCF8574x::HALValue_primary_read(Device* device, HALValue& val) {
        PCF8574x& self = static_cast<PCF8574x&>(*device);

        uint8_t received = self.wire.requestFrom(self.addr, (uint8_t)1);
        if (received == 0) return HALOperationResult::ExecutionFailed;
        val = (uint32_t)self.wire.read();
#if HAS_REACTIVE_READ(I2C_DEVICE_PCF8574X)
        self.triggerRead();
#endif
        return HALOperationResult::Success;
    }
    HALOperationResult PCF8574x::HALValue_primary_write(Device* device, const HALValue& val) {
        PCF8574x& self = static_cast<PCF8574x&>(*device);

        if (val.isNaN()) return HALOperationResult::WriteValueNaN;
        self.wire.beginTransmission(self.addr);
        self.wire.write(val.toUInt());
        uint8_t res = self.wire.endTransmission(true);
        if (res != 0) {
            // todo maybe log to global logger
            return HALOperationResult::ExecutionFailed;
        }
#if HAS_REACTIVE_WRITE(I2C_DEVICE_PCF8574X)
        self.triggerWrite();
#endif
        return HALOperationResult::Success;
    }

    void PCF8574x::PrintTo(StringBuilderStreamer& sbs) {
        Device::PrintTo(sbs);
        
        sbs.write_json_value_separator();
        sbs.write_jsonMemberStart(F("addr"));
        sbs.write_doublequote();
        sbs.write('0');
        sbs.write('x');
        sbs.write_asHex(addr);
        sbs.write_doublequote();
    }

}