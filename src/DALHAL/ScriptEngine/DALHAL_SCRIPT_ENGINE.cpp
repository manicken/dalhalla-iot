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

#include "DALHAL_SCRIPT_ENGINE.h"

#include <DALHAL/ScriptEngine/DALHAL_SCRIPT_ENGINE_Reports.h>
#include <DALHAL/Support/DALHAL_DeleterTemplate.h>

#include <DALHAL/ScriptEngine/Runtime/DALHAL_SCRIPT_ENGINE_StatementBlock.h>
#include <DALHAL/ScriptEngine/Runtime/DALHAL_SCRIPT_ENGINE_ScriptBlock.h>

#include <DALHAL/ScriptEngine/Parser/DALHAL_SCRIPT_ENGINE_Tokenizer.h>
#include <DALHAL/ScriptEngine/Parser/DALHAL_SCRIPT_ENGINE_Parser.h>
#include <DALHAL/ScriptEngine/Parser/DALHAL_SCRIPT_ENGINE_Script_Token.h>
#include <DALHAL/ScriptEngine/Parser/DALHAL_SCRIPT_ENGINE_Parser_Expressions.h>

#include <DALHAL/Support/DALHAL_Logger.h>

#include <LittleFS.h>

#if defined(ESP32) || defined(ESP8266)
#include <Support/LittleFS_ext.h>
#else
#include <LittleFS_ext.h>
#endif

namespace DALHAL {
    namespace ScriptEngine {

        bool ValidateAllActiveScripts(ScriptsToLoad& scriptsToLoad)
        {
            bool valid = true;
            int count = scriptsToLoad.scriptFileCount;
            ZeroCopyString* files = scriptsToLoad.scriptFileList;
            for (int i = 0;i<count;i++) {
                
                std::string path = DALHAL_SCRIPT_ENGINE_SCRIPTS_DIRECTORY + files[i].ToString();
                //printf("\nValidating script:%s\n",path.c_str());
                if (LittleFS.exists(path.c_str()) == false) {
                    GlobalLogger.Info(F("ValidateAllActiveScripts - script file do not exist:"), path.c_str());
                    continue;
                }
                valid = ScriptEngine::Parser::ReadAndParseScriptFile(path.c_str(), nullptr);
                if (valid == false) return false;
                //yield();
            }
            return true;
        }

        bool ValidateAndLoadAllActiveScripts()
        {
            ScriptsToLoad scriptsToLoad; // Automatically loads the scripts list file (or defaults to script.txt) on construction
            if (scriptsToLoad.scriptFileCount == 0) {
                GlobalLogger.Info(F("No scripts to load."));
                return true;
            }

            ScriptBlocks::running = false;
            ScriptEngine::Expressions::CalcStackSizesInit();
            if (ValidateAllActiveScripts(scriptsToLoad) == false) { 
                GlobalLogger.Error(F("ValidateAllActiveScripts fail!"));
                //GlobalLogger.printAllLogs(Serial, false);
                return false;
            }
            
            ScriptEngine::Expressions::InitStacks();
            if (ScriptBlocks::LoadAllActiveScripts(scriptsToLoad) == false) {
                GlobalLogger.Error(F("(SERIOUS ERROR) (SERIOUS ERROR) (SERIOUS ERROR) (SERIOUS ERROR) (SERIOUS ERROR) (SERIOUS ERROR) - could not load scripts!"));
                return false;
            }
            ScriptBlocks::running = true;
            return true;
        }
    }
}