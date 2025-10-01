/*
  Dalhalla IoT — JSON-configured HAL/DAL + Script Engine
  HAL = Hardware Abstraction Layer
  DAL = Device Abstraction Layer

  Provides IoT firmware building blocks for home automation and smart sensors.

  Copyright (C) 2025 Jannik Svensson

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
//#include "HAL_JSON_SCRIPT_ENGINE_Support.h"
#include "HAL_JSON_SCRIPT_ENGINE_StatementBlock.h"
//#include "HAL_JSON_SCRIPT_ENGINE_Parser.h"
#include "HAL_JSON_SCRIPT_ENGINE_Script_Token.h"

namespace HAL_JSON {
    namespace ScriptEngine {
        /**
         * A single trigger definition at the root level of a script.
         * Each trigger is linked to one or more executable operation blocks.
         */
        struct TriggerBlock
        {
            HAL_JSON_NOCOPY_NOMOVE(TriggerBlock);

            /** the function set to this pointer should return true if the StatementBlock(s) should execute */
            //bool (*triggerSource)();
            bool (*triggerSource)(void* context);
            void* context;  // optional — lets the triggerSource read its own state
            StatementBlock* items;
            int itemsCount;

            static bool AllwaysRun(void* context);

            TriggerBlock();
            ~TriggerBlock();

            HALOperationResult Exec();

            void Set(int statementBlockCount, ScriptTokens& tokens);
        };

        /**
         * Represents a single script file.
         * A script may contain one or more top-level triggers.
         * A script may contain IfBlocks at the top-level
         * in such cases the TriggerBlock::AllwaysRun should be used
         * as the trigger source
         */
        struct ScriptBlock
        {
            HAL_JSON_NOCOPY_NOMOVE(ScriptBlock);

            TriggerBlock* triggerBlocks;
            int triggerBlockCount;

            ScriptBlock();
            ~ScriptBlock();

            void Set(ScriptTokens& tokens);

            void Exec();
        };

        
    }
}