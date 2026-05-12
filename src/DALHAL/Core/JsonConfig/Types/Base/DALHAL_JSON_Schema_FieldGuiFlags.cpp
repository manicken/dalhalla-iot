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

            bool Gui::HaveAny(FieldGuiFlagsType flags) {
                return (flags != 0);
            }

            bool Gui::HaveAnyNotIncludingInline(FieldGuiFlagsType flags) {
                flags &= ~UseInline;
                return (flags != 0);
            }
            
            bool Gui::hasFlag(FieldGuiFlagsType flags, FieldGuiFlagsType flag) {
                return (flags & flag) != 0;
            }

            void Gui::ToJson(FieldGuiFlagsType flags, std::string& out) {
                bool first = true;
                out += '[';
                if (hasFlag(flags, DisableByDefault)) {
                    first = false;
                    ToJsonString::appendQuoted(out, F("DisableByDefault"));
                }
                
                if (hasFlag(flags, HideLabel)) {
                    if (first == false) { out += ','; }
                    first = false;
                    ToJsonString::appendQuoted(out, F("HideLabel"));
                }
                if (hasFlag(flags, ReadOnly)) {
                    if (first == false) { out += ','; }
                    first = false;
                    ToJsonString::appendQuoted(out, F("ReadOnly"));
                }
                if (hasFlag(flags, RenderAllAllowedValues)) {
                    if (first == false) { out += ','; }
                    first = false;
                    ToJsonString::appendQuoted(out, F("RenderAllAllowedValues"));
                }
                if (hasFlag(flags, UseInline)) {
                    if (first == false) { out += ','; }
                    first = false;
                    ToJsonString::appendQuoted(out, F("UseInline"));
                }
                out += ']';
            }

            bool Gui::HaveUseInline(FieldGuiFlagsType flags) {
                return (flags & UseInline) != 0;
            }
        

    }

}