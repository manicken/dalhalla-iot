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

#pragma once

#include <stdlib.h>
#include <ArduinoJson.h>

#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_TypeBase.h>
#include <DALHAL/Core/Types/DALHAL_DeviceCreateContext.h>

namespace DALHAL {

    namespace JsonSchema {

        struct ModeConjunctionDefine {
            const SchemaTypeBase* fieldRef;
            bool required; // true = must exist, false = must explicit NOT exist
        };

        using ModeApplyFn = void (*)(const DALHAL::DeviceCreateContext&, void* outStruct);

        struct ModeSelector {
            const char* name; // optional
            const ModeConjunctionDefine* conjunctions; // nullptr terminated
            ModeApplyFn apply;

            constexpr ModeSelector(const char* name, const ModeConjunctionDefine* conjunctions, ModeApplyFn apply)
                : name(name), conjunctions(conjunctions), apply(apply) {}

            static int evaluate(const ModeSelector* modes, const JsonVariant& j);
            static bool Apply(const ModeSelector* modes, const DALHAL::DeviceCreateContext&, void* outStruct);
            static void ToJson(const ModeSelector* modes, StringBuilderStreamer& sbs);
        };

        
    }

}