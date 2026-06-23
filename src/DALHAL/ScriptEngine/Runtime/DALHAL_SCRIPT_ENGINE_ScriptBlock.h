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

#include <Arduino.h>

#include "DALHAL_SCRIPT_ENGINE_StatementBlock.h"
#include "DALHAL_SCRIPT_ENGINE_TriggerBlock.h"

#include "../Parser/DALHAL_SCRIPT_ENGINE_Script_Token.h"

namespace DALHAL {
    namespace ScriptEngine {
        /**
         * Represents a single script file.
         * A script may contain one or more top-level triggers.
         * A script may contain IfBlocks at the top-level
         * in such cases the TriggerBlock::AllwaysRun should be used
         * as the trigger source
         */
        struct ScriptBlock
        {
            DALHAL_NOCOPY_NOMOVE(ScriptBlock);

            TriggerBlock* triggerBlocks;
            int triggerBlockCount;

            ScriptBlock();
            ~ScriptBlock();

            bool Set(ScriptTokens& tokens);

            void Exec();
        };

        
    }
}