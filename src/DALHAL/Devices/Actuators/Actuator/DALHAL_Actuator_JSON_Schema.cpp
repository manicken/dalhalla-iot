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

#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_Types.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_BaseTypes.h>
#include <DALHAL/Core/Manager/DALHAL_GPIO_Manager.h>
//#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>

#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Base.h>
#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Pins.h>

namespace DALHAL {

    namespace JsonSchema {

        // output pins
        constexpr FieldHardwarePin pin_hbridge_a_field = { DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_A, FieldPolicy::AllOfGroup, (GPIO_manager::PinFunc::OUT)};
        constexpr FieldHardwarePin pin_hbridge_b_field = { DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_B, FieldPolicy::AllOfGroup, (GPIO_manager::PinFunc::OUT)};
        constexpr FieldHardwarePin pin_hbridge_open_field = { DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_OPEN, FieldPolicy::AllOfGroup, (GPIO_manager::PinFunc::OUT)};
        constexpr FieldHardwarePin pin_hbridge_close_field = { DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_CLOSE, FieldPolicy::AllOfGroup, (GPIO_manager::PinFunc::OUT)};
        constexpr FieldHardwarePin pin_direnable_dir_field = { DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_DIR, FieldPolicy::AllOfGroup, (GPIO_manager::PinFunc::OUT)};
        constexpr FieldHardwarePin pin_direnable_enable_field = { DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_ENABLE, FieldPolicy::AllOfGroup, (GPIO_manager::PinFunc::OUT)};
        // this is a optional field in direnable mode
        constexpr FieldHardwarePin pin_direnable_break_field = { DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_BREAK, FieldPolicy::ModeDefine, (GPIO_manager::PinFunc::OUT)};

        // input pins
        constexpr FieldObject minEndStopField = { DALHAL_DEVICE_ACTUATOR_CFG_NAME_MIN_END_STOP, FieldPolicy::Optional, &InputPinScheme };
        constexpr FieldObject maxEndStopField = { DALHAL_DEVICE_ACTUATOR_CFG_NAME_MAX_END_STOP, FieldPolicy::Optional, &InputPinScheme };


        constexpr FieldUInt timeout_ms_field = {DALHAL_DEVICE_ACTUATOR_CFG_NAME_TIMEOUT_MS, FieldPolicy::Optional, 1, 0, DALHAL_DEVICE_ACTUATOR_CFG_DEFAULT_TIMEOUT_MS}; // default 10 seconds

        constexpr const FieldBase* hbridgeModeAB_GroupItems[] = {&pin_hbridge_a_field, &pin_hbridge_b_field, nullptr};
        constexpr AllOfGroup hbridgeModeAB_GroupFields = {"hbridgeModeAB", FieldPolicy::ModeDefine, hbridgeModeAB_GroupItems}; // here hbridgeModeAB defines what name to use for the BSON output
        
        constexpr const FieldBase* hbridgeModeOC_GroupItems[] = {&pin_hbridge_open_field, &pin_hbridge_close_field, nullptr};
        constexpr AllOfGroup hbridgeModeOC_GroupFields = {"hbridgeModeOC", FieldPolicy::ModeDefine, hbridgeModeOC_GroupItems}; // here hbridgeModeOC defines what name to use for the BSON output
        
        constexpr const FieldBase* direnableMode_GroupItems[] = {&pin_direnable_dir_field, &pin_direnable_enable_field, nullptr};
        constexpr AllOfGroup direnableMode_GroupFields = {"dir_enableMode", FieldPolicy::ModeDefine, direnableMode_GroupItems}; // here direnableMode defines what name to use for the BSON output

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
            {"h-bridge ab mode", conjunctions_hBridgeAB_Mode},
            {"h-bridge open close mode", conjunctions_hBridgeOC_Mode},
            {"dir/enable mode", conjunctions_dir_enable_Mode},
            {"dir/enable/break mode", conjunctions_dir_enable_break_Mode},
            {nullptr, nullptr}
        };
        // this list only validates each field so that it match specification
        constexpr const FieldBase* fields[] = {
            &disabled_type_uidreq_note_group, // DALHAL_CommonSchemas_Base
            &hbridgeModeAB_GroupFields,
            &hbridgeModeOC_GroupFields,
            &direnableMode_GroupFields,
            &pin_direnable_break_field,
            &minEndStopField,
            &maxEndStopField,
            &timeout_ms_field,
            nullptr
        };

        constexpr JsonObjectSchema Actuator = {
            "Actuator",
            fields,
            modes,
            nullptr, // no constraints
            EmptyPolicy::Warn,
            UnknownFieldPolicy::Warn,
        };

    }

}