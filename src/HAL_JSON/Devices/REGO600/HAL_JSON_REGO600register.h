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

#include <Arduino.h> // Needed for String class

#include <ArduinoJson.h>
#include "../../HAL_JSON_ArduinoJSON_ext.h"
#include "../../../Support/Logger.h"
#include "../../HAL_JSON_Device.h"
#include "../../HAL_JSON_Device_GlobalDefines.h"
#include "../../../Support/CharArrayHelpers.h"
#include "../../../Support/ConvertHelper.h"
//#include "../../../Drivers/REGO600.h"

#define HAL_JSON_KEYNAME_REGO600_OPCODE     "opcode"
#define HAL_JSON_KEYNAME_REGO600_ADDRESS    "addr"
#define HAL_JSON_KEYNAME_REGO600_VALUE_TYPE "valuetype"

namespace HAL_JSON {
    
    class REGO600register : public Device {
        
    public:
        enum class ValueType {
            FLOAT,
            INT,
            UINT,
            BOOL // will be returned as integer 0 or 1
        };
        //uint32_t opcode;  // need to be public for the moment
        //uint32_t address; // need to be public for the moment
        ValueType valueType;
        uint32_t value;   // need to be public for the moment
        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(const JsonVariant &jsonObj, const char* type);
        REGO600register(const JsonVariant &jsonObj, const char* type);

        HALOperationResult read(HALValue& val) override;
        String ToString() override;
    };

    inline const char* ToString(REGO600register::ValueType valueType) {
        switch (valueType) {
            case REGO600register::ValueType::BOOL: return "bool";
            case REGO600register::ValueType::FLOAT: return "float";
            case REGO600register::ValueType::INT: return "int";
            case REGO600register::ValueType::UINT: return "uint";
            default: return "Unknown";
        }
    }
}