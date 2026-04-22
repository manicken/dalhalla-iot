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

#include "DALHAL_LatchingRelay_JSON_Schema.h"

#include <DALHAL/Core/Manager/DALHAL_GPIO_Manager.h>

#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_TypeBase.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/Groups/DALHAL_JSON_Schema_FieldsGroup.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/Groups/DALHAL_JSON_Schema_AllOfFieldsGroup.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/Groups/DALHAL_JSON_Schema_OneOfFieldsGroup.h>
#include <DALHAL/Core/JsonConfig/Types/Structures/DALHAL_JSON_Schema_Object.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/DALHAL_JSON_Schema_HardwarePin.h>
#include <DALHAL/Core/JsonConfig/Types/Primitives/DALHAL_JSON_Schema_UInt.h>

#include <DALHAL/Core/JsonConfig/Types/Root/DALHAL_JSON_Schema_JsonObjectSchema.h>

#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Base.h>
#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Pins.h>

#include "DALHAL_LatchingRelay.h"

namespace DALHAL {

    namespace JsonSchema {

        namespace LatchingRelay {

            // output pins
            constexpr SchemaHardwarePin pin_direct_a_field = { "pinA", FieldPolicy::AllOfFieldsGroup, (GPIO_manager::PinFunc::OUT)};
            constexpr SchemaHardwarePin pin_direct_b_field = { "pinB", FieldPolicy::AllOfFieldsGroup, (GPIO_manager::PinFunc::OUT)};
            constexpr SchemaHardwarePin pin_direct_set_field = { "pinSet", FieldPolicy::AllOfFieldsGroup, (GPIO_manager::PinFunc::OUT)};
            constexpr SchemaHardwarePin pin_direct_reset_field = { "pinReset", FieldPolicy::AllOfFieldsGroup, (GPIO_manager::PinFunc::OUT)};
            constexpr SchemaHardwarePin pin_dataenable_data_field = { "pinData", FieldPolicy::AllOfFieldsGroup, (GPIO_manager::PinFunc::OUT)};
            constexpr SchemaHardwarePin pin_dataenable_enable_field = { "pinEnable", FieldPolicy::AllOfFieldsGroup, (GPIO_manager::PinFunc::OUT)};
            
            // input pins
            constexpr SchemaObject resetStateField = { "ResetState", FieldPolicy::Optional, &CommonPins::InputPinScheme };
            constexpr SchemaObject setStateField = { "SetState", FieldPolicy::Optional, &CommonPins::InputPinScheme };


            constexpr SchemaUInt timeout_ms_field = {"timeoutMs", FieldPolicy::Optional, (uint)1, (uint)0, (uint)500}; // default 500 mS

            constexpr const SchemaTypeBase* directModeAB_GroupItems[] = {&pin_direct_a_field, &pin_direct_b_field, nullptr};
            constexpr SchemaAllOfFieldsGroup directModeAB_GroupFields = {"directModeAB", FieldPolicy::ModeDefine, directModeAB_GroupItems }; // here hbridgeModeAB defines what name to use for the BSON output
            
            constexpr const SchemaTypeBase* directModeSR_GroupItems[] = {&pin_direct_set_field, &pin_direct_reset_field, nullptr};
            constexpr SchemaAllOfFieldsGroup directModeSR_GroupFields = {"directModeOC", FieldPolicy::ModeDefine, directModeSR_GroupItems }; // here hbridgeModeOC defines what name to use for the BSON output
            
            constexpr const SchemaTypeBase* direnableMode_GroupItems[] = {&pin_dataenable_data_field, &pin_dataenable_enable_field, nullptr};
            constexpr SchemaAllOfFieldsGroup direnableMode_GroupFields = {"dir_enableMode", FieldPolicy::ModeDefine, direnableMode_GroupItems}; // here direnableMode defines what name to use for the BSON output

            constexpr ModeConjunctionDefine conjunctions_directAB_Mode[] = {
                { &directModeAB_GroupFields, true },  // group must exist for this mode
                { &directModeSR_GroupFields, false }, // group must NOT exist for this mode
                { &direnableMode_GroupFields, false }, // group must NOT exist for this mode
                { nullptr, false}
            };

            constexpr ModeConjunctionDefine conjunctions_directSR_Mode[] = {
                { &directModeAB_GroupFields, false }, // group must NOT exist for this mode
                { &directModeSR_GroupFields, true },  // group must exist for this mode
                { &direnableMode_GroupFields, false }, // group must NOT exist for this mode
                { nullptr, false}
            };

            constexpr ModeConjunctionDefine conjunctions_data_enable_Mode[] = {
                { &directModeAB_GroupFields, false }, // group must NOT exist for this mode
                { &directModeSR_GroupFields, false }, // group must NOT exist for this mode
                { &direnableMode_GroupFields, true },  // group must exist for this mode
                { nullptr, false}
            };

            void Apply_Direct_a_b(const DeviceCreateContext& ctx, void* out);
            void Apply_Direct_set_reset(const DeviceCreateContext& ctx, void* out);
            void Apply_DataEnable(const DeviceCreateContext& ctx, void* out);
            

            constexpr const ModeSelector modes[] = {
                {"direct a b", conjunctions_directAB_Mode, Apply_Direct_a_b},
                {"direct set reset", conjunctions_directSR_Mode, Apply_Direct_set_reset},
                {"data/enable", conjunctions_data_enable_Mode, Apply_DataEnable},
                {nullptr, nullptr, nullptr}
            };
            // this list only validates each field so that it match specification
            constexpr const SchemaTypeBase* fields[] = {
                &CommonBase::disabled_type_uidreq_note_group, // DALHAL_CommonSchemas_Base
                &directModeAB_GroupFields,
                &directModeSR_GroupFields,
                &direnableMode_GroupFields,
                &resetStateField,
                &setStateField,
                &timeout_ms_field,
                nullptr
            };

            constexpr JsonObjectSchema Root = {
                "LatchingRelay",
                fields,
                modes,
                nullptr, // no constraints
                EmptyPolicy::Warn,
                UnknownFieldPolicy::Warn,
            };

            void Apply_DataEnable(const DeviceCreateContext& ctx, void* out)
            {
                auto* self = static_cast<DALHAL::LatchingRelay*>(out);
                self->pins.data_enable.data =
                    (gpio_num_t)JsonSchema::GetValue(pin_dataenable_data_field, ctx).asUInt();

                self->pins.data_enable.enable =
                    (gpio_num_t)JsonSchema::GetValue(pin_dataenable_enable_field, ctx).asUInt();

                self->mode = DALHAL::LatchingRelay::DriveMode::DataEnable;
            }

            void Apply_Direct_a_b(const DeviceCreateContext& ctx, void* out)
            {
                auto* self = static_cast<DALHAL::LatchingRelay*>(out);
                self->pins.direct.a =
                    (gpio_num_t)JsonSchema::GetValue(pin_direct_a_field, ctx).asUInt();

                self->pins.direct.b =
                    (gpio_num_t)JsonSchema::GetValue(pin_direct_b_field, ctx).asUInt();

                self->mode = DALHAL::LatchingRelay::DriveMode::Direct;
            }

            void Apply_Direct_set_reset(const DeviceCreateContext& ctx, void* out)
            {
                auto* self = static_cast<DALHAL::LatchingRelay*>(out);
                self->pins.direct.a =
                    (gpio_num_t)JsonSchema::GetValue(pin_direct_set_field, ctx).asUInt();

                self->pins.direct.b =
                    (gpio_num_t)JsonSchema::GetValue(pin_direct_reset_field, ctx).asUInt();

                self->mode = DALHAL::LatchingRelay::DriveMode::Direct;
            }
        }

    }

}