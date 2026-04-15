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
#include <DALHAL/Core/JsonConfig/Types/Logical/Groups/DALHAL_JSON_Schema_SchemaFieldsGroup.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/Groups/DALHAL_JSON_Schema_SchemaAllOfFieldsGroup.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/Groups/DALHAL_JSON_Schema_SchemaOneOfFieldsGroup.h>
#include <DALHAL/Core/JsonConfig/Types/Structures/DALHAL_JSON_Schema_Object.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/DALHAL_JSON_Schema_HardwarePin.h>
#include <DALHAL/Core/JsonConfig/Types/Primitives/DALHAL_JSON_Schema_UInt.h>

#include <DALHAL/Core/JsonConfig/Types/Root/DALHAL_JSON_Schema_JsonObjectSchema.h>

#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Base.h>
#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Pins.h>

namespace DALHAL {

    namespace JsonSchema {

        // output pins
        constexpr SchemaHardwarePin pin_direct_a_field = { DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_PIN_A, FieldPolicy::AllOfFieldsGroup, (GPIO_manager::PinFunc::OUT)};
        constexpr SchemaHardwarePin pin_direct_b_field = { DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_PIN_B, FieldPolicy::AllOfFieldsGroup, (GPIO_manager::PinFunc::OUT)};
        constexpr SchemaHardwarePin pin_direct_set_field = { DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_PIN_SET, FieldPolicy::AllOfFieldsGroup, (GPIO_manager::PinFunc::OUT)};
        constexpr SchemaHardwarePin pin_direct_reset_field = { DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_PIN_RESET, FieldPolicy::AllOfFieldsGroup, (GPIO_manager::PinFunc::OUT)};
        constexpr SchemaHardwarePin pin_direnable_dir_field = { DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_PIN_DIR, FieldPolicy::AllOfFieldsGroup, (GPIO_manager::PinFunc::OUT)};
        constexpr SchemaHardwarePin pin_direnable_enable_field = { DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_PIN_ENABLE, FieldPolicy::AllOfFieldsGroup, (GPIO_manager::PinFunc::OUT)};
        
        // input pins
        constexpr SchemaObject resetStateField = { DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_RESET_STATE, FieldPolicy::Optional, &InputPinScheme };
        constexpr SchemaObject setStateField = { DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_SET_STATE, FieldPolicy::Optional, &InputPinScheme };


        constexpr SchemaUInt timeout_ms_field = {DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_TIMEOUT_MS, FieldPolicy::Optional, 1, 0, DALHAL_DEVICE_LATCHING_RELAY_CFG_DEFAULT_TIMEOUT_MS}; // default 10 seconds

        constexpr const SchemaTypeBase* directModeAB_GroupItems[] = {&pin_direct_a_field, &pin_direct_b_field, nullptr};
        constexpr SchemaAllOfFieldsGroup directModeAB_GroupFields = {"directModeAB", FieldPolicy::ModeDefine, directModeAB_GroupItems }; // here hbridgeModeAB defines what name to use for the BSON output
        
        constexpr const SchemaTypeBase* directModeSR_GroupItems[] = {&pin_direct_set_field, &pin_direct_reset_field, nullptr};
        constexpr SchemaAllOfFieldsGroup directModeSR_GroupFields = {"directModeOC", FieldPolicy::ModeDefine, directModeSR_GroupItems }; // here hbridgeModeOC defines what name to use for the BSON output
        
        constexpr const SchemaTypeBase* direnableMode_GroupItems[] = {&pin_direnable_dir_field, &pin_direnable_enable_field, nullptr};
        constexpr SchemaAllOfFieldsGroup direnableMode_GroupFields = {"dir_enableMode", FieldPolicy::ModeDefine, direnableMode_GroupItems}; // here direnableMode defines what name to use for the BSON output

        constexpr ModeConjunctionDefine conjunctions_hBridgeAB_Mode[] = {
            { &directModeAB_GroupFields, true },  // group must exist for this mode
            { &directModeSR_GroupFields, false }, // group must NOT exist for this mode
            { &direnableMode_GroupFields, false }, // group must NOT exist for this mode
            { nullptr, false}
        };

        constexpr ModeConjunctionDefine conjunctions_hBridgeOC_Mode[] = {
            { &directModeAB_GroupFields, false }, // group must NOT exist for this mode
            { &directModeSR_GroupFields, true },  // group must exist for this mode
            { &direnableMode_GroupFields, false }, // group must NOT exist for this mode
            { nullptr, false}
        };

        constexpr ModeConjunctionDefine conjunctions_dir_enable_Mode[] = {
            { &directModeAB_GroupFields, false }, // group must NOT exist for this mode
            { &directModeSR_GroupFields, false }, // group must NOT exist for this mode
            { &direnableMode_GroupFields, true },  // group must exist for this mode
            { nullptr, false}
        };

        constexpr const ModeSelector modes[] = {
            {"direct ab mode", conjunctions_hBridgeAB_Mode},
            {"direct set reset mode", conjunctions_hBridgeOC_Mode},
            {"dir/enable mode", conjunctions_dir_enable_Mode},
            {nullptr, nullptr}
        };
        // this list only validates each field so that it match specification
        constexpr const SchemaTypeBase* fields[] = {
            &disabled_type_uidreq_note_group, // DALHAL_CommonSchemas_Base
            &directModeAB_GroupFields,
            &directModeSR_GroupFields,
            &direnableMode_GroupFields,
            &resetStateField,
            &setStateField,
            &timeout_ms_field,
            nullptr
        };

        constexpr JsonObjectSchema LatchingRelay = {
            "LatchingRelay",
            fields,
            modes,
            nullptr, // no constraints
            EmptyPolicy::Warn,
            UnknownFieldPolicy::Warn,
        };

    }

}