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

#include "DALHAL_PWM_Servo_JSON_Schema.h"

#include <DALHAL/Core/Manager/DALHAL_GPIO_Manager.h>


#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_TypeBase.h>
#include <DALHAL/Core/JsonConfig/Types/Root/DALHAL_JSON_Schema_ModeSelector.h>
#include <DALHAL/Core/JsonConfig/Types/Root/DALHAL_JSON_Schema_FieldConstraint.h>
#include <DALHAL/Core/JsonConfig/Types/Primitives/DALHAL_JSON_Schema_UInt.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/DALHAL_JSON_Schema_HardwarePin.h>
#include <DALHAL/Core/JsonConfig/Types/Primitives/DALHAL_JSON_Schema_Float.h>


#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Base.h>
#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Pins.h>

#include "DALHAL_PWM_Servo.h"

namespace DALHAL {

    namespace JsonSchema {

        namespace PWM_Servo {

            //constexpr SchemaHardwarePin pinField = { DALHAL_COMMON_CFG_NAME_PIN, FieldPolicy::Required, (GPIO_manager::PinFunc::OUT)};

            constexpr SchemaUInt chField = {"ch", FieldPolicy::Required, (uint)0, (uint)7, (uint)0};

            constexpr SchemaUInt minPulseLengthField = {"minPulseLength", FieldPolicy::Optional, (uint)100, (uint)20000, (uint)1000};
            constexpr SchemaUInt maxPulseLengthField = {"maxPulseLength", FieldPolicy::Optional, (uint)100, (uint)20000, (uint)2000};
            constexpr SchemaUInt startPulseLengthField = {"startPulseLength", FieldPolicy::Optional, (uint)100, (uint)20000, (uint)1500};

            constexpr SchemaUInt autoOffAfterMsField = {"autoOffAfterMs", FieldPolicy::Optional, (uint)0, (uint)0, (uint)0};
            constexpr SchemaUInt pulseLengthOffsetField = {"pulseLengthOffset", FieldPolicy::Optional, (uint)0, (uint)0, (uint)0};

            constexpr SchemaFloat minValField = {"minVal", FieldPolicy::ModeDefine, (uint)0};
            constexpr SchemaFloat maxValField = {"maxVal", FieldPolicy::ModeDefine, (uint)100};

            constexpr ModeConjunctionDefine conjunctions_ratio_value_Mode[] = {
                { &minValField, true }, // field must exist for this mode
                { &maxValField, true }, // field must exist for this mode
                { nullptr, false}
            };

            constexpr ModeConjunctionDefine conjunctions_pulselen_value_Mode[] = {
                { &minValField, false }, // field must NOT exist for this mode
                { &maxValField, false }, // field must NOT exist for this mode
                { nullptr, false}
            };

            void Apply_RatioValueMode(const DeviceCreateContext& ctx, void* out);
            void Apply_PulseLengthValueMode(const DeviceCreateContext& ctx, void* out);

            constexpr const ModeSelector modes[] = {
                {"ratio value", conjunctions_ratio_value_Mode, Apply_RatioValueMode},
                {"pulse length", conjunctions_pulselen_value_Mode, Apply_PulseLengthValueMode},
                {nullptr, nullptr, nullptr}
            };

            constexpr FieldConstraint constraints[] = {
                {&minValField, FieldConstraint::Type::LessThan, &maxValField},
                {&minPulseLengthField, FieldConstraint::Type::LessThan, &maxPulseLengthField},
                {&minPulseLengthField, FieldConstraint::Type::LessThanOrEqual, &startPulseLengthField},
                {&startPulseLengthField, FieldConstraint::Type::LessThanOrEqual, &maxPulseLengthField},
                {nullptr, FieldConstraint::Type::Void, nullptr}
            };

            constexpr const SchemaTypeBase* fields[] = {
                &CommonBase::disabled_type_uidreq_note_group, // DALHAL_CommonSchemas_Base
                &CommonPins::OutputPinField,
                &chField,
                &minPulseLengthField, // actually pulse length cfg fields are individually optional
                &maxPulseLengthField,
                &startPulseLengthField,
                &autoOffAfterMsField,
                &pulseLengthOffsetField,
                &minValField,
                &maxValField,
                nullptr,
            };

            constexpr JsonObjectSchema Root = {
                "PWM_Servo",
                fields,
                modes,
                constraints,
                EmptyPolicy::Warn,
                UnknownFieldPolicy::Warn,
            };


            void Apply_RatioValueMode(const DeviceCreateContext& ctx, void* out)
            {
                auto* self = static_cast<DALHAL::PWM_Servo*>(out);
                self->valueType = DALHAL::PWM_Servo::ServoValueType::Ratio;
                self->minVal = JsonSchema::GetValue(minValField, ctx).asFloat();
                self->maxVal = JsonSchema::GetValue(maxValField, ctx).asFloat();
            }

            void Apply_PulseLengthValueMode(const DeviceCreateContext& ctx, void* out)
            {
                auto* self = static_cast<DALHAL::PWM_Servo*>(out);
                self->valueType = DALHAL::PWM_Servo::ServoValueType::PulseUS;
                self->minVal = NAN;
                self->maxVal = NAN;
            }
        }

    }

}