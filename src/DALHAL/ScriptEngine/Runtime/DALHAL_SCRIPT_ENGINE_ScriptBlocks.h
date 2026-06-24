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

#include <DALHAL/ScriptEngine/Parser/DALHAL_SCRIPT_ENGINE_Script_Token.h> // ScriptTokens
#include <DALHAL/ScriptEngine/Runtime/DALHAL_SCRIPT_ENGINE_ScriptBlock.h> // ScriptBlock
#include <DALHAL/ScriptEngine/Runtime/DALHAL_SCRIPT_ENGINE_ScriptsToLoad.h>

namespace DALHAL {
    namespace ScriptEngine {
        
        /**
         * Global container for all loaded scripts in the engine.
         * This is the highest-level structure in the script engine hierarchy.
         */
        struct ScriptBlocks
        {
            /** set this to true to make the script run,
             * set to false to make the script not run 
             * this is set to false if any of the script validate
             * fails
             * */
            static bool running;
            static ScriptBlock* scriptBlocks;
            static int scriptBlocksCount;
            static int currentScriptIndex;

            /** just a callback wrapper to begin initializing the structures */
            static bool ScriptFileParsed(ScriptTokens& tokens);
            
            /** ValidateAllActiveScripts should be run before using this function */
            static bool LoadAllActiveScripts(ScriptsToLoad& scriptsToLoad);
            
            /** entry point of one script loop iteraction */
            static void Exec();
        };
    }
}