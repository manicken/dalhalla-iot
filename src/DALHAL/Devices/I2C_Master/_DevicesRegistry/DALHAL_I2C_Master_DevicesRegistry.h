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

#pragma once

#include <DALHAL/Core/Device/DALHAL_Device.h>

#include <Wire.h>

#include <DALHAL/Core/Types/DALHAL_Registry.h>

#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_BaseTypes.h>

namespace DALHAL {

    typedef bool (*I2C_HAL_DEVICE_HAS_ADDR_FUNC)(uint8_t addr);

    struct I2C_RegistryDefine : public Registry::DefineBase {
        I2C_HAL_DEVICE_HAS_ADDR_FUNC HasAddress_Function;

        constexpr I2C_RegistryDefine(
            Registry::HAL_DEVICE_CREATE_FUNC Create_Function, 
            const JsonSchema::JsonObjectSchema* jsonSchema,
            I2C_HAL_DEVICE_HAS_ADDR_FUNC HasAddress_Function
        ) : 
            Registry::DefineBase(Create_Function, jsonSchema),
            HasAddress_Function(HasAddress_Function)
        {}

        constexpr I2C_RegistryDefine(
            Registry::HAL_DEVICE_CREATE_FUNC Create_Function, 
            const JsonSchema::JsonObjectSchema* jsonSchema,
            const EventDescriptor* reactiveTable,
            I2C_HAL_DEVICE_HAS_ADDR_FUNC HasAddress_Function
        ) : 
            Registry::DefineBase(Create_Function, jsonSchema, reactiveTable),
            HasAddress_Function(HasAddress_Function)
        {}

    };

    extern const Registry::Item I2C_DeviceRegistry[];
    
    // used by i2c scanner to describe which devices a adress belongs to
    std::string describeI2CAddress(uint8_t addr);

    struct I2C_Master_CreateFunctionContext : DeviceCreateContext {
        TwoWire& wire;
        I2C_Master_CreateFunctionContext(TwoWire& wire) : DeviceCreateContext(), wire(wire) {}
    };

}