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

#include <DALHAL/ScriptEngine/Runtime/IfStatement/DALHAL_SCRIPT_ENGINE_BranchBlock.h>

namespace DALHAL {
    namespace ScriptEngine {

        // forward declaration
        struct ScriptTokens;

        struct ConditionalBranch : public BranchBlock
        {
            DALHAL_NOCOPY_NOMOVE(ConditionalBranch);

            /** is either LogicExecNode or CalcCompareRPN (pure without logic) */
            void* context;
            /** used to delete the context depending on type */
            Deleter deleter;
            HALOperationResult (*handler)(void* context);

            ConditionalBranch();
            ~ConditionalBranch();

            bool Set(ScriptTokens& tokens);
        };

    }
}