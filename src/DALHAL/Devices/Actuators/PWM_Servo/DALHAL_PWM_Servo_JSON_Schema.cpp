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

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_Types.h>
#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_BaseTypes.h>
#include <DALHAL/Core/Manager/DALHAL_GPIO_Manager.h>

#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Base.h>
#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Pins.h>

namespace DALHAL {

    namespace JsonSchema {

        constexpr FieldHardwarePin pinField = { DALHAL_COMMON_CFG_NAME_PIN, FieldPolicy::Required, static_cast<uint8_t>(GPIO_manager::PinFunc::OUT)};

        constexpr FieldUInt chField = {DALHAL_DEVICE_PWM_SERVO_CFG_NAME_CH, FieldPolicy::Required, 0, 7, 0};

        constexpr FieldUInt minPulseLengthField = {"minPulseLength", FieldPolicy::Optional, 100, 20000, 1000};
        constexpr FieldUInt maxPulseLengthField = {"maxPulseLength", FieldPolicy::Optional, 100, 20000, 2000};
        constexpr FieldUInt startPulseLengthField = {"startPulseLength", FieldPolicy::Optional, 100, 20000, 1500};

        constexpr FieldUInt autoOffAfterMsField = {"autoOffAfterMs", FieldPolicy::Optional, 0, 0, 0};
        constexpr FieldUInt pulseLengthOffsetField = {"pulseLengthOffset", FieldPolicy::Optional, 0, 0, 0};

        constexpr FieldFloat minValField = {"minVal", FieldPolicy::Optional, 0, 0, 0};
        constexpr FieldFloat maxValField = {"maxVal", FieldPolicy::Optional, 0, 0, 0};

        constexpr FieldConstraint constraints[] = {
            {&minValField, FieldConstraint::Type::LessThan, &maxValField},
            {&minPulseLengthField, FieldConstraint::Type::LessThan, &maxPulseLengthField},
            {&minPulseLengthField, FieldConstraint::Type::LessThanOrEqual, &startPulseLengthField},
            {&startPulseLengthField, FieldConstraint::Type::LessThanOrEqual, &maxPulseLengthField},
            {nullptr, FieldConstraint::Type::Void, nullptr}
        };

        constexpr const FieldBase* fields[] = {
            &typeField,         // DALHAL_CommonSchemas_Base
            &uidFieldRequired,  // DALHAL_CommonSchemas_Base
            &pinField,
            &chField,
            &minPulseLengthField, // actually pulse length cfg fields are individually optional
            &maxPulseLengthField,
            &startPulseLengthField,
            &autoOffAfterMsField,
            &pulseLengthOffsetField,
            &minValField,// min and max value defined must be in a group as they must be defined together
            &maxValField,
            nullptr,
        };

        constexpr JsonObjectSchema PWM_ServoDevice = {
            "PWM_Servo",
            fields,
            nullptr, // no modes right now
            constraints,
            EmptyPolicy::Warn,
            UnknownFieldPolicy::Warn,
        };

    }

}