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

#include "DALHAL_LatchingRelay_JSON_Scheme.h"

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema.h>
#include <DALHAL/Core/JsonConfig/DALHAL_JSON_SchemaFieldBase.h>
#include <DALHAL/Core/Manager/DALHAL_GPIO_Manager.h>
#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>
#include <DALHAL/Core/JsonConfig/CommonSchemes/DALHAL_CommonSchemes_Pins.h>

namespace DALHAL {

    // output pins
    constexpr JsonSchema::FieldHardwarePin pin_direct_a_field = { DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_PIN_A, JsonSchema::FieldFlag::AllOfGroup, static_cast<uint8_t>(GPIO_manager::PinFunc::OUT)};
    constexpr JsonSchema::FieldHardwarePin pin_direct_b_field = { DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_PIN_B, JsonSchema::FieldFlag::AllOfGroup, static_cast<uint8_t>(GPIO_manager::PinFunc::OUT)};
    constexpr JsonSchema::FieldHardwarePin pin_direct_set_field = { DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_PIN_SET, JsonSchema::FieldFlag::AllOfGroup, static_cast<uint8_t>(GPIO_manager::PinFunc::OUT)};
    constexpr JsonSchema::FieldHardwarePin pin_direct_reset_field = { DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_PIN_RESET, JsonSchema::FieldFlag::AllOfGroup, static_cast<uint8_t>(GPIO_manager::PinFunc::OUT)};
    constexpr JsonSchema::FieldHardwarePin pin_direnable_dir_field = { DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_PIN_DIR, JsonSchema::FieldFlag::AllOfGroup, static_cast<uint8_t>(GPIO_manager::PinFunc::OUT)};
    constexpr JsonSchema::FieldHardwarePin pin_direnable_enable_field = { DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_PIN_ENABLE, JsonSchema::FieldFlag::AllOfGroup, static_cast<uint8_t>(GPIO_manager::PinFunc::OUT)};
    
    // input pins
    constexpr JsonSchema::FieldObject resetStateField = { DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_RESET_STATE, JsonSchema::FieldFlag::Optional, &InputPinScheme };
    constexpr JsonSchema::FieldObject setStateField = { DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_SET_STATE, JsonSchema::FieldFlag::Optional, &InputPinScheme };


    constexpr JsonSchema::FieldUInt timeout_ms_field = {DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_TIMEOUT_MS, JsonSchema::FieldFlag::Optional, 1, 0, DALHAL_DEVICE_LATCHING_RELAY_CFG_DEFAULT_TIMEOUT_MS}; // default 10 seconds

    constexpr const JsonSchema::FieldBase* directModeAB_GroupItems[] = {&pin_direct_a_field, &pin_direct_b_field, nullptr};
    constexpr JsonSchema::AllOfGroup directModeAB_GroupFields = {"directModeAB", JsonSchema::FieldFlag::ModeDefine, hbridgeModeAB_GroupItems}; // here hbridgeModeAB defines what name to use for the BSON output
    
    constexpr const JsonSchema::FieldBase* directModeSR_GroupItems[] = {&pin_direct_set_field, &pin_direct_reset_field, nullptr};
    constexpr JsonSchema::AllOfGroup directModeSR_GroupFields = {"directModeOC", JsonSchema::FieldFlag::ModeDefine, hbridgeModeOC_GroupItems}; // here hbridgeModeOC defines what name to use for the BSON output
    
    constexpr const JsonSchema::FieldBase* direnableMode_GroupItems[] = {&pin_direnable_dir_field, &pin_direnable_enable_field, nullptr};
    constexpr JsonSchema::AllOfGroup direnableMode_GroupFields = {"dir_enableMode", JsonSchema::FieldFlag::ModeDefine, direnableMode_GroupItems}; // here direnableMode defines what name to use for the BSON output

    constexpr JsonSchema::ModeConjunctionDefine conjunctions_hBridgeAB_Mode[] = {
        { &directModeAB_GroupFields, true },  // group must exist for this mode
        { &directModeSR_GroupFields, false }, // group must NOT exist for this mode
        { &direnableMode_GroupFields, false }, // group must NOT exist for this mode
        { nullptr, false}
    };

    constexpr JsonSchema::ModeConjunctionDefine conjunctions_hBridgeOC_Mode[] = {
        { &directModeAB_GroupFields, false }, // group must NOT exist for this mode
        { &directModeSR_GroupFields, true },  // group must exist for this mode
        { &direnableMode_GroupFields, false }, // group must NOT exist for this mode
        { nullptr, false}
    };

    constexpr JsonSchema::ModeConjunctionDefine conjunctions_dir_enable_Mode[] = {
        { &directModeAB_GroupFields, false }, // group must NOT exist for this mode
        { &directModeSR_GroupFields, false }, // group must NOT exist for this mode
        { &direnableMode_GroupFields, true },  // group must exist for this mode
        { nullptr, false}
    };

    constexpr const JsonSchema::ModeSelector modes[] = {
        {"direct ab mode", conjunctions_hBridgeAB_Mode},
        {"direct set reset mode", conjunctions_hBridgeOC_Mode},
        {"dir/enable mode", conjunctions_dir_enable_Mode},
        {nullptr, nullptr}
    };
    // this list only validates each field so that it match specification
    constexpr const JsonSchema::FieldBase* fields[] = {
        &JsonSchema::typeField, // include this here as otherwise we will get unknown field found
        &JsonSchema::uidFieldRequired,
        &directModeAB_GroupFields,
        &directModeSR_GroupFields,
        &direnableMode_GroupFields,
        &resetStateField,
        &setStateField,
        &timeout_ms_field,
        nullptr
    };

    constexpr JsonSchema::JsonObjectScheme JsonObjectSchemeLatchingRelayDevice = {
        "LatchingRelay",
        fields,
        modes,
        nullptr // no constraints
    };

}