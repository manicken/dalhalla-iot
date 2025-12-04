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

#pragma once

#include "../../HAL_JSON_Device.h"
#include <Wire.h>

namespace HAL_JSON {

    typedef Device* (*I2C_HAL_DEVICE_CREATE_FUNC)(const JsonVariant &json, const char* type, TwoWire& wire);
    typedef bool (*I2C_HAL_DEVICE_VERIFY_JSON_FUNC)(const JsonVariant &json);
    typedef bool (*I2C_HAL_DEVICE_HAS_ADDR_FUNC)(uint8_t addr);

    typedef struct I2C_DeviceRegistryDefine {
        I2C_HAL_DEVICE_CREATE_FUNC Create_Function;
        I2C_HAL_DEVICE_VERIFY_JSON_FUNC Verify_JSON_Function;
        I2C_HAL_DEVICE_HAS_ADDR_FUNC HasAddress_Function;
    } I2C_DeviceRegistryDefine;

    typedef struct I2C_DeviceRegistryItem {
        const char* typeName;
        I2C_DeviceRegistryDefine def;
    } I2C_DeviceRegistryItem ;

    extern const I2C_DeviceRegistryItem I2C_DeviceRegistry[];
    const I2C_DeviceRegistryItem& GetI2C_DeviceTypeDef(const char* type);
    // used by i2c scanner to describe which devices a adress belongs to
    std::string describeI2CAddress(uint8_t addr);

}