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

#include "DALHAL_SinglePulseOutput_JSON_Schema.h"

#include <DALHAL/Core/Manager/DALHAL_GPIO_Manager.h>

#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_TypeBase.h>
#include <DALHAL/Core/JsonConfig/Types/Primitives/DALHAL_JSON_Schema_UInt.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/String/DALHAL_JSON_Schema_StringAnyOfArrayConstrained.h> // also ByArrayConstraints
#include <DALHAL/Core/JsonConfig/Types/Logical/DALHAL_JSON_Schema_HardwarePin.h>
#include <DALHAL/Core/JsonConfig/Types/Root/DALHAL_JSON_Schema_JsonObjectSchema.h>

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h> // DALHAL_KEYNAME_SINGLE_PULSE_OUTPUT_DEFAULT_PULSE_LENGHT

#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Base.h>
#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Pins.h>

#include "DALHAL_SinglePulseOutput.h"

namespace DALHAL {

    namespace JsonSchema {

        namespace SinglePulseOutput {

            constexpr SchemaHardwarePin pinField = { DALHAL_COMMON_CFG_NAME_PIN, FieldPolicy::Required, (GPIO_manager::PinFunc::IN) };
            constexpr SchemaUInt pulseLengthField = { DALHAL_KEYNAME_SINGLE_PULSE_OUTPUT_DEFAULT_PULSE_LENGHT, FieldPolicy::Optional, (unsigned int)1, (unsigned int)0, (unsigned int)500};
            
            //constexpr ByArrayConstraints activeLevelConstraints = {CommonPins::activeLevelStrings, ByArrayConstraints::Policy::IgnoreCase};
            //constexpr SchemaStringAnyOfArrayConstrained activeLevelField = { "activeLevel", FieldPolicy::Optional, DALHAL_COMMON_CFG_VALUE_PIN_LEVEL_HIGH, &activeLevelConstraints};
            constexpr SchemaUInt activeLevelField = {"activeLevel", FieldPolicy::Optional, (unsigned int)0, (unsigned int)1, (unsigned int)0}; 

            constexpr const SchemaTypeBase* fields[] = {
                &CommonBase::disabled_type_uidreq_note_group, // DALHAL_CommonSchemas_Base
                &pinField,
                &pulseLengthField,
                &activeLevelField,
                nullptr,
            };

            constexpr JsonObjectSchema Root = {
                "SinglePulseOutput",
                fields,
                nullptr, // no modes
                nullptr,  // no constraints
                EmptyPolicy::Warn,
                UnknownFieldPolicy::Warn,
            };

            void Extractors::Apply(const DALHAL::DeviceCreateContext& context, DALHAL::SinglePulseOutput* out) {
                out->uid = encodeUID(JsonSchema::GetValue(JsonSchema::CommonBase::uidFieldRequired, context).asConstChar());
                out->pin = JsonSchema::GetValue(JsonSchema::SinglePulseOutput::pinField, context);
                out->activeLevel = JsonSchema::GetValue(JsonSchema::SinglePulseOutput::activeLevelField, context);
                out->pulseLength = JsonSchema::GetValue(JsonSchema::SinglePulseOutput::pulseLengthField, context);
        
            }

        }

    }

}