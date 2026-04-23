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

            constexpr SchemaUInt chField = {"ch", FieldPolicy::Required, (unsigned int)0, (unsigned int)7, (unsigned int)0};

            constexpr SchemaUInt minPulseLengthField = {"minPulseLength", FieldPolicy::Optional, (unsigned int)100, (unsigned int)20000, (unsigned int)1000};
            constexpr SchemaUInt maxPulseLengthField = {"maxPulseLength", FieldPolicy::Optional, (unsigned int)100, (unsigned int)20000, (unsigned int)2000};
            constexpr SchemaUInt startPulseLengthField = {"startPulseLength", FieldPolicy::Optional, (unsigned int)100, (unsigned int)20000, (unsigned int)1500};

            constexpr SchemaUInt autoOffAfterMsField = {"autoOffAfterMs", FieldPolicy::Optional, (unsigned int)0, (unsigned int)0, (unsigned int)0};
            constexpr SchemaUInt pulseLengthOffsetField = {"pulseLengthOffset", FieldPolicy::Optional, (unsigned int)0, (unsigned int)0, (unsigned int)0};

            constexpr SchemaFloat minValField = {"minVal", FieldPolicy::ModeDefine, (unsigned int)0};
            constexpr SchemaFloat maxValField = {"maxVal", FieldPolicy::ModeDefine, (unsigned int)100};

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

            constexpr const ModeSelector modes[] = {
                {"ratio value", conjunctions_ratio_value_Mode, Extractors::Apply_RatioValueMode},
                {"pulse length", conjunctions_pulselen_value_Mode, Extractors::Apply_PulseLengthValueMode},
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

            void Extractors::Apply_RatioValueMode(const DeviceCreateContext& ctx, void* out)
            {
                auto* self = static_cast<DALHAL::PWM_Servo*>(out);
                self->valueType = DALHAL::PWM_Servo::ServoValueType::Ratio;
                self->minVal = JsonSchema::GetValue(minValField, ctx).asFloat();
                self->maxVal = JsonSchema::GetValue(maxValField, ctx).asFloat();
            }

            void Extractors::Apply_PulseLengthValueMode(const DeviceCreateContext& ctx, void* out)
            {
                auto* self = static_cast<DALHAL::PWM_Servo*>(out);
                self->valueType = DALHAL::PWM_Servo::ServoValueType::PulseUS;
                self->minVal = NAN;
                self->maxVal = NAN;
            }

            void Extractors::Apply(const DALHAL::DeviceCreateContext& context, DALHAL::PWM_Servo* out) {
                out->uid = encodeUID(JsonSchema::GetValue(JsonSchema::CommonBase::uidFieldRequired, context).asConstChar());
                out->pin = JsonSchema::GetValue(JsonSchema::CommonPins::OutputPinField, context);
                out->pwmChannel = (ledc_channel_t)JsonSchema::GetValue(JsonSchema::PWM_Servo::chField, context).asUInt();

                out->minPulseLength = JsonSchema::GetValue(JsonSchema::PWM_Servo::minPulseLengthField, context);
                out->maxPulseLength = JsonSchema::GetValue(JsonSchema::PWM_Servo::maxPulseLengthField, context);
                out->startPulseLength = JsonSchema::GetValue(JsonSchema::PWM_Servo::startPulseLengthField, context);
                out->autoOffAfterMs = JsonSchema::GetValue(JsonSchema::PWM_Servo::autoOffAfterMsField, context);
                out->pulseLengthOffset = JsonSchema::GetValue(JsonSchema::PWM_Servo::pulseLengthOffsetField, context);

                JsonSchema::ModeSelector::Apply(JsonSchema::PWM_Servo::Root.modes, context, out);
            }

        }

    }

}