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

#include "DALHAL_TX433_Unit_TypeSFC_JSON_Schema.h"

#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_TypeBase.h>
#include <DALHAL/Core/JsonConfig/Types/Primitives/DALHAL_JSON_Schema_UInt.h>
#include <DALHAL/Core/JsonConfig/Types/Root/DALHAL_JSON_Schema_JsonObjectSchema.h>

#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Base.h>

#include <DALHAL/Drivers/RF433.h>
#include "DALHAL_TX433_Unit.h"

namespace DALHAL {

    namespace JsonSchema {

        namespace TX433_Unit_TypeSFC {

            constexpr SchemaUInt chField = {"ch", FieldPolicy::Optional, (unsigned int)1, (unsigned int)4, (unsigned int)0};
            constexpr SchemaUInt btnField = {"btn", FieldPolicy::Optional, (unsigned int)1, (unsigned int)4, (unsigned int)0};
            constexpr SchemaUInt stateField = {"state", FieldPolicy::Optional, (unsigned int)0, (unsigned int)1, (unsigned int)0};

            constexpr const SchemaTypeBase* fields[] = {
                &CommonBase::disabled_type_uidreq_note_group, // DALHAL_CommonSchemas_Base
                &chField,
                &btnField,
                &stateField,
                nullptr,
            };

            constexpr JsonObjectSchema Root = {
                "TX433_Unit_TypeSFC",
                fields,
                nullptr, // no modes
                nullptr,  // no constraints
                EmptyPolicy::Warn,
                UnknownFieldPolicy::Warn,
            };

            void Extractors::Apply(DALHAL::DeviceCreateContext& context, DALHAL::TX433_Unit* out) {
                out->staticData = RF433::Get433_SFC_Data(*(context.jsonObjItem));
                out->model = TX433_MODEL::FixedCode;
                out->fixedState = (*context.jsonObjItem).containsKey(stateField.name);
            }

        }

    }

}