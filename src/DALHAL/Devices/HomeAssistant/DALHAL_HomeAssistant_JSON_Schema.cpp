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

#include "DALHAL_HomeAssistant_JSON_Schema.h"

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_Types.h>
#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_BaseTypes.h>

#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Base.h>
#include "DALHAL_HA_DeviceTypeReg.h"

namespace DALHAL {

    namespace JsonSchema {

        constexpr FieldString deviceIdField = {"deviceId", FieldPolicy::Required, nullptr, 0};
        constexpr FieldString hostField = {"host", FieldPolicy::Required, nullptr, 0};
        constexpr FieldUInt   portField = {"port", FieldPolicy::Required, 1, 65535, 1883};
        constexpr FieldString userField = {"user", FieldPolicy::AllOfGroup, nullptr, 0};
        constexpr FieldString passField = {"pass", FieldPolicy::AllOfGroup, nullptr, 0};

        constexpr const FieldBase* credentialsFields[] = {&userField, &passField, nullptr};
        constexpr AllOfGroup credentialsGroup = {"credentials", FieldPolicy::Optional, credentialsFields};

        constexpr FieldString groupNameField = {"name", FieldPolicy::Required, nullptr, 0};

        constexpr const FieldBase* globalGroupFields[] = {&uidFieldRequired, &groupNameField, nullptr};
        constexpr JsonObjectSchema globalGroupSchema = {
            "GlobalGroup",
            globalGroupFields,
            nullptr, // no modes
            nullptr,  // no constraints
            EmptyPolicy::Warn,
            UnknownFieldPolicy::Warn,
        };
        constexpr FieldObject globalGroupField = {"group", FieldPolicy::ModeDefine, &globalGroupSchema};

        constexpr FieldRegistryArray itemsField = {"items", FieldPolicy::ModeDefine, HA_DeviceRegistry, "ROOT.HOMEASSISTANT"};

        constexpr const FieldBase* individualGroupFields[] = {&uidFieldRequired, &groupNameField, &itemsField, nullptr};
        constexpr JsonObjectSchema individualGroupSchema = {
            "IndividualGroup",
            individualGroupFields,
            nullptr, // no modes
            nullptr,  // no constraints
            EmptyPolicy::Warn,
            UnknownFieldPolicy::Warn,
        };
        constexpr FieldArray individualGroupsField = {"groups", FieldPolicy::ModeDefine, &individualGroupSchema};

        constexpr const FieldBase* fields[] = {
            &typeField,         // DALHAL_CommonSchemas_Base
            &uidFieldRequired,  // DALHAL_CommonSchemas_Base
            &deviceIdField,
            &hostField,
            &portField,
            &credentialsGroup,
            &globalGroupField,
            &itemsField,
            &individualGroupsField,
            nullptr,
        };

        constexpr ModeConjunctionDefine globalGroupModeConjunctions[] = {
            { &globalGroupField, true },      // group must exist for this mode
            { &itemsField, true },            // items must exist
            { &individualGroupsField, false },// groups must NOT exist
            { nullptr, false}
        };

        constexpr ModeConjunctionDefine individualGroupModeConjunctions[] = {
            { &globalGroupField, false },      // group must NOT exist for this mode
            { &itemsField, false },            // items must NOT exist
            { &individualGroupsField, true },// groups must exist
            { nullptr, false}
        };

        constexpr ModeSelector consumerDeviceModes[] = {
            {"global group mode", globalGroupModeConjunctions},
            {"individual groups mode", individualGroupModeConjunctions},
            {nullptr, nullptr}
        };

        constexpr JsonObjectSchema HomeAssistant = {
            "HomeAssistant",
            fields,
            consumerDeviceModes,
            nullptr,  // no constraints
            EmptyPolicy::Warn,
            UnknownFieldPolicy::Warn,
        };

    }

}