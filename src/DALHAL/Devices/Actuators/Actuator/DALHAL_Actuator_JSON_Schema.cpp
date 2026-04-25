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

#include "DALHAL_Actuator_JSON_Schema.h"

#include <DALHAL/Core/Manager/DALHAL_GPIO_Manager.h>

#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_TypeBase.h>
#include <DALHAL/Core/JsonConfig/Types/Root/DALHAL_JSON_Schema_ModeSelector.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/Groups/DALHAL_JSON_Schema_FieldsGroup.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/Groups/DALHAL_JSON_Schema_AllOfFieldsGroup.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/Groups/DALHAL_JSON_Schema_OneOfFieldsGroup.h>
#include <DALHAL/Core/JsonConfig/Types/Structures/DALHAL_JSON_Schema_Object.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/DALHAL_JSON_Schema_HardwarePin.h>
#include <DALHAL/Core/JsonConfig/Types/Primitives/DALHAL_JSON_Schema_UInt.h>

#include <DALHAL/Core/JsonConfig/Types/Root/DALHAL_JSON_Schema_JsonObjectSchema.h>

#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Base.h>
#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Pins.h>

#include "DALHAL_Actuator.h"

namespace DALHAL {

    namespace JsonSchema {

        namespace Actuator {

            // output pins
            constexpr SchemaHardwarePin pin_hbridge_a_field = { "pinA", FieldPolicy::AllOfFieldsGroup, (GPIO_manager::PinFunc::OUT)};
            constexpr SchemaHardwarePin pin_hbridge_b_field = { "pinB", FieldPolicy::AllOfFieldsGroup, (GPIO_manager::PinFunc::OUT)};
            constexpr SchemaHardwarePin pin_hbridge_open_field = { "pinOpen", FieldPolicy::AllOfFieldsGroup, (GPIO_manager::PinFunc::OUT)};
            constexpr SchemaHardwarePin pin_hbridge_close_field = { "pinClose", FieldPolicy::AllOfFieldsGroup, (GPIO_manager::PinFunc::OUT)};
            constexpr SchemaHardwarePin pin_direnable_dir_field = { "pinDir", FieldPolicy::AllOfFieldsGroup, (GPIO_manager::PinFunc::OUT)};
            constexpr SchemaHardwarePin pin_direnable_enable_field = { "pinEnable", FieldPolicy::AllOfFieldsGroup, (GPIO_manager::PinFunc::OUT)};
            // this is a optional field in direnable mode
            constexpr SchemaHardwarePin pin_direnable_break_field = { "pinBreak", FieldPolicy::ModeDefine, (GPIO_manager::PinFunc::OUT)};

            // input pins
            constexpr SchemaObject minEndStopField = { "MinEndStop", FieldPolicy::Optional, &CommonPins::InputPinScheme };
            constexpr SchemaObject maxEndStopField = { "MaxEndStop", FieldPolicy::Optional, &CommonPins::InputPinScheme };


            constexpr SchemaUInt timeout_ms_field = {"timeoutMs", FieldPolicy::Optional, (unsigned int)1, (unsigned int)0, (unsigned int)10000}; // default 10 seconds

            constexpr const SchemaTypeBase* hbridgeModeAB_GroupItems[] = {&pin_hbridge_a_field, &pin_hbridge_b_field, nullptr};
            constexpr SchemaAllOfFieldsGroup hbridgeModeAB_GroupFields = {"hbridgeModeAB", FieldPolicy::ModeDefine, hbridgeModeAB_GroupItems}; // here hbridgeModeAB defines what name to use for the BSON output
            
            constexpr const SchemaTypeBase* hbridgeModeOC_GroupItems[] = {&pin_hbridge_open_field, &pin_hbridge_close_field, nullptr};
            constexpr SchemaAllOfFieldsGroup hbridgeModeOC_GroupFields = {"hbridgeModeOC", FieldPolicy::ModeDefine, hbridgeModeOC_GroupItems}; // here hbridgeModeOC defines what name to use for the BSON output
            
            constexpr const SchemaTypeBase* direnableMode_GroupItems[] = {&pin_direnable_dir_field, &pin_direnable_enable_field, nullptr};
            constexpr SchemaAllOfFieldsGroup direnableMode_GroupFields = {"dir_enableMode", FieldPolicy::ModeDefine, direnableMode_GroupItems}; // here direnableMode defines what name to use for the BSON output

            constexpr ModeConjunctionDefine conjunctions_hBridgeAB_Mode[] = {
                { &hbridgeModeAB_GroupFields, true },  // group must exist for this mode
                { &hbridgeModeOC_GroupFields, false }, // group must NOT exist for this mode
                { &direnableMode_GroupFields, false }, // group must NOT exist for this mode
                { &pin_direnable_break_field, false }, // field must NOT exist for this mode
                { nullptr, false}
            };

            constexpr ModeConjunctionDefine conjunctions_hBridgeOC_Mode[] = {
                { &hbridgeModeAB_GroupFields, false }, // group must NOT exist for this mode
                { &hbridgeModeOC_GroupFields, true },  // group must exist for this mode
                { &direnableMode_GroupFields, false }, // group must NOT exist for this mode
                { &pin_direnable_break_field, false }, // field must NOT exist for this mode
                { nullptr, false}
            };

            constexpr ModeConjunctionDefine conjunctions_dir_enable_Mode[] = {
                { &hbridgeModeAB_GroupFields, false }, // group must NOT exist for this mode
                { &hbridgeModeOC_GroupFields, false }, // group must NOT exist for this mode
                { &direnableMode_GroupFields, true },  // group must exist for this mode
                { &pin_direnable_break_field, false }, // field must NOT exist for this mode
                { nullptr, false}
            };

            constexpr ModeConjunctionDefine conjunctions_dir_enable_break_Mode[] = {
                { &hbridgeModeAB_GroupFields, false }, // group must NOT exist for this mode
                { &hbridgeModeOC_GroupFields, false }, // group must NOT exist for this mode
                { &direnableMode_GroupFields, true },  // group must exist for this mode
                { &pin_direnable_break_field, true },  // field must exist for this mode
                { nullptr, false}
            };

            constexpr const ModeSelector modes[] = {
                {"h-bridge ab", conjunctions_hBridgeAB_Mode, Extractors::Apply_HBridge_a_b},
                {"h-bridge open close", conjunctions_hBridgeOC_Mode, Extractors::Apply_HBridge_open_close},
                {"dir/enable", conjunctions_dir_enable_Mode, Extractors::Apply_DirEnableBreak},
                {"dir/enable/break", conjunctions_dir_enable_break_Mode, Extractors::Apply_DirEnableBreak},
                {nullptr, nullptr, nullptr}
            };
            // this list only validates each field so that it match specification
            constexpr const SchemaTypeBase* fields[] = {
                &CommonBase::disabled_type_uidreq_note_group, // DALHAL_CommonSchemas_Base
                &hbridgeModeAB_GroupFields,
                &hbridgeModeOC_GroupFields,
                &direnableMode_GroupFields,
                &pin_direnable_break_field,
                &minEndStopField,
                &maxEndStopField,
                &timeout_ms_field,
                nullptr
            };

            constexpr JsonObjectSchema Root = {
                "Actuator",
                fields,
                modes,
                nullptr, // no constraints
                EmptyPolicy::Warn,
                UnknownFieldPolicy::Warn,
            };

            void Extractors::Apply_DirEnableBreak(const DeviceCreateContext& ctx, void* out)
            {
                auto* self = static_cast<DALHAL::Actuator*>(out);

                self->pins.diren.dir = (gpio_num_t)pin_direnable_dir_field.ExtractFrom(*ctx.jsonObjItem);
                self->pins.diren.enable = (gpio_num_t)pin_direnable_enable_field.ExtractFrom(*ctx.jsonObjItem);
                self->pins.diren.brk =  (gpio_num_t)pin_direnable_break_field.ExtractFrom(*ctx.jsonObjItem);

                self->mode = DALHAL::Actuator::DriveMode::DirEnable;
            }

            void Extractors::Apply_HBridge_a_b(const DeviceCreateContext& ctx, void* out)
            {
                auto* self = static_cast<DALHAL::Actuator*>(out);

                self->pins.hbridge.a = (gpio_num_t)pin_hbridge_a_field.ExtractFrom(*ctx.jsonObjItem);
                self->pins.hbridge.b = (gpio_num_t)pin_hbridge_b_field.ExtractFrom(*ctx.jsonObjItem);

                self->mode = DALHAL::Actuator::DriveMode::HBridge;
            }

            void Extractors::Apply_HBridge_open_close(const DeviceCreateContext& ctx, void* out)
            {
                auto* self = static_cast<DALHAL::Actuator*>(out);

                self->pins.hbridge.a = (gpio_num_t)pin_hbridge_open_field.ExtractFrom(*ctx.jsonObjItem);
                self->pins.hbridge.b = (gpio_num_t)pin_hbridge_close_field.ExtractFrom(*ctx.jsonObjItem);

                self->mode = DALHAL::Actuator::DriveMode::HBridge;
            }

            void Extractors::Apply(const DALHAL::DeviceCreateContext& context, DALHAL::Actuator* out) {
                out->uid = encodeUID(JsonSchema::CommonBase::uidFieldRequired.ExtractFrom(*context.jsonObjItem));

                JsonSchema::ModeSelector::Apply(JsonSchema::Actuator::Root.modes, context, out);

                JsonSchema::PinConfig endStopPinCfg;
                if (JsonSchema::SchemaObject::ExtractValues(JsonSchema::Actuator::minEndStopField, *(context.jsonObjItem), &endStopPinCfg)) {
                    out->pinMinEndStop = (gpio_num_t)endStopPinCfg.pin;
                    out->pinMinEndStopActiveHigh = endStopPinCfg.activeHigh;
                } else {
                    out->pinMinEndStop = gpio_num_t::GPIO_NUM_NC;
                }
                if (JsonSchema::SchemaObject::ExtractValues(JsonSchema::Actuator::maxEndStopField, *(context.jsonObjItem), &endStopPinCfg)) {
                    out->pinMaxEndStop = (gpio_num_t)endStopPinCfg.pin;
                    out->pinMaxEndStopActiveHigh = endStopPinCfg.activeHigh;
                } else {
                    out->pinMaxEndStop = gpio_num_t::GPIO_NUM_NC;
                }

                out->timeoutMs = JsonSchema::Actuator::timeout_ms_field.ExtractFrom(*context.jsonObjItem);
            }
        }

    }

}