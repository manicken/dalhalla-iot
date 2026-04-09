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

#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_TypeBase.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_ComplexTypes.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_Array.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_Object.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_StringBase.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_UInt.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_RegistryArray.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_JsonObjectSchema.h>

#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Base.h>
#include "DALHAL_HA_DeviceTypeReg.h"

namespace DALHAL {

    namespace JsonSchema {

        constexpr SchemaStringBase deviceIdField = {"deviceId", FieldPolicy::Required};
        constexpr SchemaStringBase hostField = {"host", FieldPolicy::Required};
        constexpr SchemaUInt   portField = {"port", FieldPolicy::Required, 1, 65535, 1883};
        constexpr SchemaStringBase userField = {"user", FieldPolicy::AllOfFieldsGroup};
        constexpr SchemaStringBase passField = {"pass", FieldPolicy::AllOfFieldsGroup};

        constexpr const SchemaTypeBase* credentialsFields[] = {&userField, &passField, nullptr};
        constexpr SchemaAllOfFieldsGroup credentialsGroup = {"credentials", FieldPolicy::Optional, credentialsFields};

        constexpr SchemaStringBase groupNameField = {"name", FieldPolicy::Required};

        constexpr const SchemaTypeBase* globalGroupFields[] = {&uidFieldRequired, &groupNameField, nullptr};
        constexpr JsonObjectSchema globalGroupSchema = {
            "GlobalGroup",
            globalGroupFields,
            nullptr, // no modes
            nullptr,  // no constraints
            EmptyPolicy::Warn,
            UnknownFieldPolicy::Warn,
        };
        constexpr SchemaObject globalGroupField = {"group", FieldPolicy::ModeDefine, &globalGroupSchema};

        constexpr SchemaRegistryArray itemsField = {"items", FieldPolicy::ModeDefine, HA_DeviceRegistry, "ROOT.HOMEASSISTANT"};

        constexpr const SchemaTypeBase* individualGroupFields[] = {&uidFieldRequired, &groupNameField, &itemsField, nullptr};
        constexpr JsonObjectSchema individualGroupSchema = {
            "IndividualGroup",
            individualGroupFields,
            nullptr, // no modes
            nullptr,  // no constraints
            EmptyPolicy::Warn,
            UnknownFieldPolicy::Warn,
        };
        constexpr SchemaArray individualGroupsField = {"groups", FieldPolicy::ModeDefine, &individualGroupSchema};

        constexpr const SchemaTypeBase* fields[] = {
            &disabled_type_uidreq_note_group, // DALHAL_CommonSchemas_Base
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