/*
  Dalhalla IoT — JSON-configured HAL/DAL + Script Engine
  HAL = Hardware Abstraction Layer
  DAL = Device Abstraction Layer

  Provides IoT firmware building blocks for home automation and smart sensors.

  Copyright (C) 2025 Jannik Svensson

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

#include "DALHAL_Actuator_JSON_Scheme.h"

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema.h>
#include <DALHAL/Core/JsonConfig/DALHAL_JSON_SchemaFieldBase.h>
#include <DALHAL/Core/Manager/DALHAL_GPIO_Manager.h>
#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>
#include <DALHAL/Core/JsonConfig/CommonSchemes/DALHAL_CommonSchemes_Pins.h>

namespace DALHAL {

    // output pins
    constexpr JsonSchema::FieldHardwarePin pin_hbridge_a_field = { DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_A, JsonSchema::FieldFlag::AllOfGroup, static_cast<uint8_t>(GPIO_manager::PinFunc::OUT)};
    constexpr JsonSchema::FieldHardwarePin pin_hbridge_b_field = { DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_B, JsonSchema::FieldFlag::AllOfGroup, static_cast<uint8_t>(GPIO_manager::PinFunc::OUT)};
    constexpr JsonSchema::FieldHardwarePin pin_hbridge_open_field = { DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_OPEN, JsonSchema::FieldFlag::AllOfGroup, static_cast<uint8_t>(GPIO_manager::PinFunc::OUT)};
    constexpr JsonSchema::FieldHardwarePin pin_hbridge_close_field = { DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_CLOSE, JsonSchema::FieldFlag::AllOfGroup, static_cast<uint8_t>(GPIO_manager::PinFunc::OUT)};
    constexpr JsonSchema::FieldHardwarePin pin_direnable_dir_field = { DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_DIR, JsonSchema::FieldFlag::AllOfGroup, static_cast<uint8_t>(GPIO_manager::PinFunc::OUT)};
    constexpr JsonSchema::FieldHardwarePin pin_direnable_enable_field = { DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_ENABLE, JsonSchema::FieldFlag::AllOfGroup, static_cast<uint8_t>(GPIO_manager::PinFunc::OUT)};
    // this is a optional field in direnable mode
    constexpr JsonSchema::FieldHardwarePin pin_direnable_break_field = { DALHAL_DEVICE_ACTUATOR_CFG_NAME_PIN_BREAK, JsonSchema::FieldFlag::ModeDefine, static_cast<uint8_t>(GPIO_manager::PinFunc::OUT)};

    // input pins
    constexpr JsonSchema::FieldObject minEndStopField = { DALHAL_DEVICE_ACTUATOR_CFG_NAME_MIN_END_STOP, JsonSchema::FieldFlag::Optional, &InputPinScheme };
    constexpr JsonSchema::FieldObject maxEndStopField = { DALHAL_DEVICE_ACTUATOR_CFG_NAME_MAX_END_STOP, JsonSchema::FieldFlag::Optional, &InputPinScheme };


    constexpr JsonSchema::FieldUInt timeout_ms_field = {DALHAL_DEVICE_ACTUATOR_CFG_NAME_TIMEOUT_MS, JsonSchema::FieldFlag::Optional, 1, 0, DALHAL_DEVICE_ACTUATOR_CFG_DEFAULT_TIMEOUT_MS}; // default 10 seconds

    constexpr const JsonSchema::FieldBase* hbridgeModeAB_GroupItems[] = {&pin_hbridge_a_field, &pin_hbridge_b_field, nullptr};
    constexpr JsonSchema::AllOfGroup hbridgeModeAB_GroupFields = {"hbridgeModeAB", JsonSchema::FieldFlag::ModeDefine, hbridgeModeAB_GroupItems}; // here hbridgeModeAB defines what name to use for the BSON output
    
    constexpr const JsonSchema::FieldBase* hbridgeModeOC_GroupItems[] = {&pin_hbridge_open_field, &pin_hbridge_close_field, nullptr};
    constexpr JsonSchema::AllOfGroup hbridgeModeOC_GroupFields = {"hbridgeModeOC", JsonSchema::FieldFlag::ModeDefine, hbridgeModeOC_GroupItems}; // here hbridgeModeOC defines what name to use for the BSON output
    
    constexpr const JsonSchema::FieldBase* direnableMode_GroupItems[] = {&pin_direnable_dir_field, &pin_direnable_enable_field, nullptr};
    constexpr JsonSchema::AllOfGroup direnableMode_GroupFields = {"dir_enableMode", JsonSchema::FieldFlag::ModeDefine, direnableMode_GroupItems}; // here direnableMode defines what name to use for the BSON output

    constexpr JsonSchema::ModeConjunctionDefine conjunctions_hBridgeAB_Mode[] = {
        { &hbridgeModeAB_GroupFields, true },  // group must exist for this mode
        { &hbridgeModeOC_GroupFields, false }, // group must NOT exist for this mode
        { &direnableMode_GroupFields, false }, // group must NOT exist for this mode
        { &pin_direnable_break_field, false }, // field must NOT exist for this mode
        { nullptr, false}
    };

    constexpr JsonSchema::ModeConjunctionDefine conjunctions_hBridgeOC_Mode[] = {
        { &hbridgeModeAB_GroupFields, false }, // group must NOT exist for this mode
        { &hbridgeModeOC_GroupFields, true },  // group must exist for this mode
        { &direnableMode_GroupFields, false }, // group must NOT exist for this mode
        { &pin_direnable_break_field, false }, // field must NOT exist for this mode
        { nullptr, false}
    };

    constexpr JsonSchema::ModeConjunctionDefine conjunctions_dir_enable_Mode[] = {
        { &hbridgeModeAB_GroupFields, false }, // group must NOT exist for this mode
        { &hbridgeModeOC_GroupFields, false }, // group must NOT exist for this mode
        { &direnableMode_GroupFields, true },  // group must exist for this mode
        { &pin_direnable_break_field, false }, // field must NOT exist for this mode
        { nullptr, false}
    };

    constexpr JsonSchema::ModeConjunctionDefine conjunctions_dir_enable_break_Mode[] = {
        { &hbridgeModeAB_GroupFields, false }, // group must NOT exist for this mode
        { &hbridgeModeOC_GroupFields, false }, // group must NOT exist for this mode
        { &direnableMode_GroupFields, true },  // group must exist for this mode
        { &pin_direnable_break_field, true },  // field must exist for this mode
        { nullptr, false}
    };

    constexpr const JsonSchema::ModeSelector modes[] = {
        {"h-bridge ab mode", conjunctions_hBridgeAB_Mode},
        {"h-bridge open close mode", conjunctions_hBridgeOC_Mode},
        {"dir/enable mode", conjunctions_dir_enable_Mode},
        {"dir/enable/break mode", conjunctions_dir_enable_break_Mode},
        {nullptr, nullptr}
    };
    // this list only validates each field so that it match specification
    constexpr const JsonSchema::FieldBase* fields[] = {
        &JsonSchema::typeField, // include this here as otherwise we will get unknown field found
        &JsonSchema::uidFieldRequired,
        &hbridgeModeAB_GroupFields,
        &hbridgeModeOC_GroupFields,
        &direnableMode_GroupFields,
        &pin_direnable_break_field,
        &minEndStopField,
        &maxEndStopField,
        &timeout_ms_field,
        nullptr
    };

    constexpr JsonSchema::JsonObjectScheme JsonObjectSchemeActuatorDevice = {
        "ActuatorDevice",
        fields,
        modes,
        nullptr // no constraints
    };

}