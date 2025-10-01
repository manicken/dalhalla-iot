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

#include "HAL_JSON_SCRIPT_ENGINE_Script_Token.h"
#include "HAL_JSON_SCRIPT_ENGINE_Script.h"

namespace HAL_JSON {
    namespace ScriptEngine {
        struct ScriptsToLoad {
            char* scriptsListContents;
            ZeroCopyString* scriptFileList;
            int scriptFileCount;
            /** On creation, this tries to load the scripts list file from LittleFS.
              * If the file exists and is valid, it tokenizes the contents and initializes the script list.
              * Otherwise, it falls back to a default script file (script.txt).
            */
            ScriptsToLoad();
            ~ScriptsToLoad();

            void InitScriptList(int count);

            // Delete copy constructor/assignment to prevent double-free
            ScriptsToLoad(const ScriptsToLoad&) = delete;
            ScriptsToLoad& operator=(const ScriptsToLoad&) = delete;
        };
        /**
         * Global container for all loaded scripts in the engine.
         * This is the highest-level structure in the script engine hierarchy.
         */
        struct ScriptsBlock
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
            static void ScriptFileParsed(ScriptTokens& tokens);
            
            /** ValidateAllActiveScripts should be run before using this function */
            static bool LoadAllActiveScripts(ScriptsToLoad& scriptsToLoad);
            
        };
        /** should be run before using LoadAllActiveScripts */
        bool ValidateAllActiveScripts(ScriptsToLoad& scriptsToLoad);
        /** begins with validating all scripts
         * and if all pass then it begins to load in the structures
         */
        bool ValidateAndLoadAllActiveScripts();
        /** entry point of one script loop iteraction */
        void Exec();
    }
}