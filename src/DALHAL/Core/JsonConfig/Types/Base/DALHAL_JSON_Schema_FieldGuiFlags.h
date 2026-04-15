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

#include <stdlib.h>

namespace DALHAL {

    namespace JsonSchema {

        using FieldGuiFlags = uint8_t;
        namespace Gui {
            constexpr FieldGuiFlags None                   = 0;
            constexpr FieldGuiFlags UseInline              = 1 << 0;
            constexpr FieldGuiFlags RenderAllAllowedValues = 1 << 1;
            constexpr FieldGuiFlags DisableByDefault       = 1 << 2;
            constexpr FieldGuiFlags ReadOnly               = 1 << 3;
            constexpr FieldGuiFlags HideLabel              = 1 << 4;

            constexpr bool hasFlag(FieldGuiFlags flags, FieldGuiFlags flag) {
                return (flags & flag) != 0;
            }
        }

    }

}