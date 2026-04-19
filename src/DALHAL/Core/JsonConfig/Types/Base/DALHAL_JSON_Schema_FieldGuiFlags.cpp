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

#include "DALHAL_JSON_Schema_FieldGuiFlags.h"
#include <string>
#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_ToJsonStringHelpers.h>

namespace DALHAL {

    namespace JsonSchema {

        namespace Gui {
            
            bool hasFlag(FieldGuiFlags flags, FieldGuiFlags flag) {
                return (flags & flag) != 0;
            }

            void ToJson(FieldGuiFlags flags, std::string& out) {

                ToJsonString::appendBool(out, "DisableByDefault", Gui::hasFlag(flags, Gui::DisableByDefault));
                out += ','; ToJsonString::appendBool(out, "HideLabel", Gui::hasFlag(flags, Gui::HideLabel));
                out += ','; ToJsonString::appendBool(out, "ReadOnly", Gui::hasFlag(flags, Gui::ReadOnly));
                out += ','; ToJsonString::appendBool(out, "RenderAllAllowedValues", Gui::hasFlag(flags, Gui::RenderAllAllowedValues));
                out += ','; ToJsonString::appendBool(out, "UseInline", Gui::hasFlag(flags, Gui::UseInline));
            }

            constexpr bool HaveUseInline(FieldGuiFlags flags) {
                return (flags & UseInline) != 0;
            }
        }

    }

}