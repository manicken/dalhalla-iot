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

#include "DALHAL_Display_SSD1306_Element.h"

#include <DALHAL/Support/DALHAL_Logger.h>

#include "DALHAL_Display_SSD1306_Element_JSON_Schema.h"

namespace DALHAL {

    __attribute__((used, externally_visible))
    constexpr Registry::DefineBase Display_SSD1306_Element::RegistryDefine = {
        nullptr,
        &JsonSchema::Display_SSD1306_Element::Root,
        &Display_SSD1306_Element::FunctionTable,
    };

    /* override */
    const Registry::DefineBase* Display_SSD1306_Element::GetRegistryDefine() {
        return &Display_SSD1306_Element::RegistryDefine;
    }

    constexpr FunctionEntry<FunctionTypes::ReadToHALValue> Display_SSD1306_Element::readValueFunctions[] = {
        DALHAL_PRIMARY_FUNCTION_ENTRY(HALValue_primary_read, "read back the latest write value")
    };

    constexpr FunctionEntry<FunctionTypes::WriteHALValue> Display_SSD1306_Element::writeValueFunctions[] = {
        DALHAL_PRIMARY_FUNCTION_ENTRY(HALValue_primary_write, "write value"),
    };

    __attribute__((used, externally_visible))
    constexpr DeviceFunctionTable Display_SSD1306_Element::FunctionTable = {
        EmptyFunctionTable<FunctionTypes::Exec>,
        DALHAL_FUNCTION_TABLE_ENTRY(readValueFunctions),
        DALHAL_FUNCTION_TABLE_ENTRY(writeValueFunctions),
        EmptyFunctionTable<FunctionTypes::BracketOpRead>,
        EmptyFunctionTable<FunctionTypes::BracketOpWrite>,
        EmptyFunctionTable<FunctionTypes::ReadString>,
        EmptyFunctionTable<FunctionTypes::WriteString>,
    };
    
    Display_SSD1306_Element::Display_SSD1306_Element(DeviceCreateContext& context) : Device(context.deviceType) {
        JsonSchema::Display_SSD1306_Element::Extractors::Apply(context, this);
    }

    void Display_SSD1306_Element::PrintTo(StringBuilderStreamer& sbs) {
        Device::PrintTo(sbs);
    }

    HALOperationResult Display_SSD1306_Element::HALValue_primary_read(Device* device, HALValue& val) {
        val = static_cast<Display_SSD1306_Element&>(*device).val;
        return HALOperationResult::Success;
    }

    HALOperationResult Display_SSD1306_Element::HALValue_primary_write(Device* device, const HALValue& val) {
        if (val.isNaN()) return HALOperationResult::WriteValueNaN;
        static_cast<Display_SSD1306_Element&>(*device).val = val;
        return HALOperationResult::Success;
    }

}