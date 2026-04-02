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

#include "DALHAL_DHT_JSON_Schema.h"

#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_Types.h>
#include <DALHAL/Core/JsonConfig/Types/DALHAL_JSON_Schema_BaseTypes.h>

#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Base.h>
#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Pins.h>
#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Time.h>

namespace DALHAL {

    namespace JsonSchema {

        constexpr const char* models[] = {
            DALHAL_TYPE_DHT_MODEL_DHT11, 
            DALHAL_TYPE_DHT_MODEL_DHT22, 
            DALHAL_TYPE_DHT_MODEL_AM2302,
            DALHAL_TYPE_DHT_MODEL_RHT03,
            nullptr
        };
        constexpr ByArrayConstraints modelFieldConstraints = {models, ByArrayConstraints::Policy::IgnoreCase};
        constexpr const FieldStringAnyOfArrayConstrained modelField = {"model", FieldPolicy::Required, DALHAL_TYPE_DHT_MODEL_DHT11, &modelFieldConstraints};

        constexpr const FieldBase* fields[] = {
            &disabled_type_uidreq_note_group, // DALHAL_CommonSchemas_Base
            &modelField,
            &refreshTimeGroupFieldsRequired,
            &InputOutputPinField,
            nullptr,
        };

        constexpr JsonObjectSchema DHT = {
            "DHT",
            fields,
            nullptr, // no modes
            nullptr,  // no constraints
            EmptyPolicy::Error,
            UnknownFieldPolicy::Warn,
        };

    }

}