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

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_Types.h>
#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_BaseTypes.h>
#include <DALHAL/Core/Manager/DALHAL_GPIO_Manager.h>
#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>

#include <DALHAL/Core/JsonConfig/CommonSchemes/DALHAL_CommonSchemes_Base.h>
#include <DALHAL/Core/JsonConfig/CommonSchemes/DALHAL_CommonSchemes_Pins.h>

namespace DALHAL {

    namespace JsonSchema {

        // output pins
        constexpr FieldHardwarePin pin_direct_a_field = { DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_PIN_A, FieldFlag::AllOfGroup, static_cast<uint8_t>(GPIO_manager::PinFunc::OUT)};
        constexpr FieldHardwarePin pin_direct_b_field = { DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_PIN_B, FieldFlag::AllOfGroup, static_cast<uint8_t>(GPIO_manager::PinFunc::OUT)};
        constexpr FieldHardwarePin pin_direct_set_field = { DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_PIN_SET, FieldFlag::AllOfGroup, static_cast<uint8_t>(GPIO_manager::PinFunc::OUT)};
        constexpr FieldHardwarePin pin_direct_reset_field = { DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_PIN_RESET, FieldFlag::AllOfGroup, static_cast<uint8_t>(GPIO_manager::PinFunc::OUT)};
        constexpr FieldHardwarePin pin_direnable_dir_field = { DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_PIN_DIR, FieldFlag::AllOfGroup, static_cast<uint8_t>(GPIO_manager::PinFunc::OUT)};
        constexpr FieldHardwarePin pin_direnable_enable_field = { DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_PIN_ENABLE, FieldFlag::AllOfGroup, static_cast<uint8_t>(GPIO_manager::PinFunc::OUT)};
        
        // input pins
        constexpr FieldObject resetStateField = { DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_RESET_STATE, FieldFlag::Optional, &InputPinScheme };
        constexpr FieldObject setStateField = { DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_SET_STATE, FieldFlag::Optional, &InputPinScheme };


        constexpr FieldUInt timeout_ms_field = {DALHAL_DEVICE_LATCHING_RELAY_CFG_NAME_TIMEOUT_MS, FieldFlag::Optional, 1, 0, DALHAL_DEVICE_LATCHING_RELAY_CFG_DEFAULT_TIMEOUT_MS}; // default 10 seconds

        constexpr const FieldBase* directModeAB_GroupItems[] = {&pin_direct_a_field, &pin_direct_b_field, nullptr};
        constexpr AllOfGroup directModeAB_GroupFields = {"directModeAB", FieldFlag::ModeDefine, hbridgeModeAB_GroupItems}; // here hbridgeModeAB defines what name to use for the BSON output
        
        constexpr const FieldBase* directModeSR_GroupItems[] = {&pin_direct_set_field, &pin_direct_reset_field, nullptr};
        constexpr AllOfGroup directModeSR_GroupFields = {"directModeOC", FieldFlag::ModeDefine, hbridgeModeOC_GroupItems}; // here hbridgeModeOC defines what name to use for the BSON output
        
        constexpr const FieldBase* direnableMode_GroupItems[] = {&pin_direnable_dir_field, &pin_direnable_enable_field, nullptr};
        constexpr AllOfGroup direnableMode_GroupFields = {"dir_enableMode", FieldFlag::ModeDefine, direnableMode_GroupItems}; // here direnableMode defines what name to use for the BSON output

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
        constexpr const FieldBase* fields[] = {
            &typeField,        // DALHAL_CommonSchemes_Base.h
            &uidFieldRequired, // DALHAL_CommonSchemes_Base.h
            &directModeAB_GroupFields,
            &directModeSR_GroupFields,
            &direnableMode_GroupFields,
            &resetStateField,
            &setStateField,
            &timeout_ms_field,
            nullptr
        };

        constexpr JsonObjectScheme LatchingRelayDevice = {
            "LatchingRelay",
            fields,
            modes,
            nullptr // no constraints
        };

    }

}