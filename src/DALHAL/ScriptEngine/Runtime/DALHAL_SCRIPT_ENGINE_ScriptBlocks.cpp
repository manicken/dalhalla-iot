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

#include "DALHAL_SCRIPT_ENGINE_ScriptBlocks.h"

#include <DALHAL/Support/DALHAL_Logger.h>

#include <DALHAL/ScriptEngine/Parser/DALHAL_SCRIPT_ENGINE_Script_Token.h> // ScriptTokens
#include <DALHAL/ScriptEngine/Runtime/DALHAL_SCRIPT_ENGINE_ScriptBlock.h> // ScriptBlock
#include <DALHAL/ScriptEngine/Runtime/DALHAL_SCRIPT_ENGINE_ScriptsToLoad.h>
#include <DALHAL/ScriptEngine/Parser/DALHAL_SCRIPT_ENGINE_Parser.h>

#include <DALHAL/ScriptEngine/DALHAL_SCRIPT_ENGINE_Reports.h>

#include <LittleFS.h>

namespace DALHAL {
    namespace ScriptEngine {

        /* static */
        void ScriptBlocks::Exec() {
           // printf("\033[2J\033[H");  // clear screen + move cursor to top-left
#if defined(_WIN32) || defined(__linux__)
            //printf("\n****** SCRIPT LOOP START *******\n");
#endif
            for (int i=0;i<ScriptBlocks::scriptBlocksCount;i++) {
                ScriptBlocks::scriptBlocks[i].Exec();
            }
#if defined(_WIN32) || defined(__linux__)
           // printf("\n****** SCRIPT LOOP END *******\n");
#endif
            
        }
        bool ScriptBlocks::running = false;
        ScriptBlock* ScriptBlocks::scriptBlocks = nullptr;
        int ScriptBlocks::scriptBlocksCount = 0;
        int ScriptBlocks::currentScriptIndex = 0;

        bool ScriptBlocks::ScriptFileParsed(ScriptTokens& tokens) {
            ReportInfo("\n");
            ReportInfo("**************************************************************************************\n");
            ReportInfo("**************************************************************************************\n");
            ReportInfo("**                                                                                  **\n");
            ReportInfo("** ##       ######   #####  ######      #######  ###### ######  ## ######  ######## **\n");
            ReportInfo("** ##      ##    ## ##   ## ##   ##     ##      ##      ##   ## ## ##   ##    ##    **\n");
            ReportInfo("** ##      ##    ## ####### ##   ##     ####### ##      ######  ## ######     ##    **\n");
            ReportInfo("** ##      ##    ## ##   ## ##   ##          ## ##      ##   ## ## ##         ##    **\n");
            ReportInfo("** #######  ######  ##   ## ######      #######  ###### ##   ## ## ##         ##    **\n");
            ReportInfo("**                                                                                  **\n");
            ReportInfo("**************************************************************************************\n");
            ReportInfo("**************************************************************************************\n");

            bool anyError = (scriptBlocks[currentScriptIndex].Set(tokens) == false);

            ReportInfo("**************************************************************************************\n");
            ReportInfo("**************************************************************************************\n");
#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
            printf("\n\ntokens.currIndex(%d) of tokens.count(%d) reached end of 'script'\n\n", tokens.currIndex, tokens.count);
#endif
            ReportInfo("**************************************************************************************\n");
            ReportInfo("**************************************************************************************\n");
            return (anyError == false);
        }


        bool ScriptBlocks::LoadAllActiveScripts(ScriptsToLoad& scriptsToLoad)
        {
            delete[] scriptBlocks;
            scriptBlocks = nullptr;
            scriptBlocksCount = 0;
            int count = scriptsToLoad.scriptFileCount;
            ZeroCopyString* files = scriptsToLoad.scriptFileList;

            scriptBlocks = new ScriptBlock[count];
            scriptBlocksCount = count;
            bool valid = true; // absolute failsafe
            for (int i = 0;i<count;i++) {
                currentScriptIndex = i;
                // this should now pass and execute the given callback
                std::string path = DALHAL_SCRIPT_ENGINE_SCRIPTS_DIRECTORY + files[i].ToString();
                if (LittleFS.exists(path.c_str()) == false) {
                    GlobalLogger.Info(F("LoadAllActiveScripts - script file do not exist:"), path.c_str());
                    continue;
                }
                valid = ScriptEngine::Parser::ReadAndParseScriptFile(path.c_str(), ScriptFileParsed);
                if (valid == false) return false;
                //yield();
            }
            return true;
        }


    }
}