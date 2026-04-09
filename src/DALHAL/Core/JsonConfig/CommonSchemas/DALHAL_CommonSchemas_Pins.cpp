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

#include "DALHAL_CommonSchemas_Pins.h"

#include <DALHAL/Core/Manager/DALHAL_GPIO_Manager.h>

#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_Bool.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_JsonObjectSchema.h>

namespace DALHAL {

    namespace JsonSchema {
        
        constexpr SchemaHardwarePin InputOutputPinField = { DALHAL_COMMON_CFG_NAME_PIN, FieldPolicy::Required, (GPIO_manager::PinFunc::IN | GPIO_manager::PinFunc::OUT) };


        constexpr SchemaHardwarePin InputPinField = { DALHAL_COMMON_CFG_NAME_PIN, FieldPolicy::Required, (GPIO_manager::PinFunc::IN) };
        constexpr SchemaBool InputActiveHighField = { DALHAL_COMMON_CFG_NAME_PIN_ACTIVE_HIGH, FieldPolicy::Optional, false };
        constexpr const SchemaTypeBase* InputFields[] = { &InputPinField, &InputActiveHighField, nullptr };
        constexpr JsonObjectSchema InputPinScheme = { "InputPinScheme", InputFields, nullptr, nullptr };

        constexpr SchemaHardwarePin OutputPinField = { DALHAL_COMMON_CFG_NAME_PIN, FieldPolicy::Required, (GPIO_manager::PinFunc::OUT) };
        constexpr SchemaBool OutputActiveHighField = { DALHAL_COMMON_CFG_NAME_PIN_ACTIVE_HIGH, FieldPolicy::Optional, false };
        constexpr const SchemaTypeBase* OutputFields[] = { &OutputPinField, &OutputActiveHighField, nullptr };
        constexpr JsonObjectSchema OutputPinScheme = { "OutputPinScheme", OutputFields, nullptr, nullptr };

    }
    
}