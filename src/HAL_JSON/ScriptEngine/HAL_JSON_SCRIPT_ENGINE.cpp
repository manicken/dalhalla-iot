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

#include "HAL_JSON_SCRIPT_ENGINE.h"

#include "HAL_JSON_SCRIPT_ENGINE_Support.h"
#include "HAL_JSON_SCRIPT_ENGINE_StatementBlock.h"
#include "HAL_JSON_SCRIPT_ENGINE_Parser.h"
#include "HAL_JSON_SCRIPT_ENGINE_Parser_Token.h"
#include "HAL_JSON_SCRIPT_ENGINE_Script.h"

#if defined(_WIN32) || defined(__linux__)
#define DEMO_SCRIPT_FILE_PATH "script1.txt"
#else
#define DEMO_SCRIPT_FILE_PATH "/scripts/script1.txt"
#endif
namespace HAL_JSON {
    namespace ScriptEngine {

        void Exec() {
           // printf("\033[2J\033[H");  // clear screen + move cursor to top-left
#if defined(_WIN32) || defined(__linux__)
            printf("\n****** SCRIPT LOOP START *******\n");
#endif
            for (int i=0;i<ScriptsBlock::scriptBlocksCount;i++) {
                ScriptsBlock::scriptBlocks[i].Exec();
            }
#if defined(_WIN32) || defined(__linux__)
            printf("\n****** SCRIPT LOOP END *******\n");
#endif
            
        }
        bool ScriptsBlock::running = false;
        ScriptBlock* ScriptsBlock::scriptBlocks = nullptr;
        int ScriptsBlock::scriptBlocksCount = 0;
        int ScriptsBlock::currentScriptIndex = 0;

        bool ValidateAllActiveScripts()
        {
            bool valid = true;
            valid = ScriptEngine::Parser::ReadAndParseScriptFile(DEMO_SCRIPT_FILE_PATH, nullptr);
            return valid;
        }

        void ScriptsBlock::ScriptFileParsed(Tokens& tokens) {
            Expressions::ReportInfo("\n");
            Expressions::ReportInfo("**************************************************************************************\n");
            Expressions::ReportInfo("**************************************************************************************\n");
            Expressions::ReportInfo("**                                                                                  **\n");
            Expressions::ReportInfo("** ██       ██████   █████  ██████      ███████  ██████ ██████  ██ ██████  ████████ **\n");
            Expressions::ReportInfo("** ██      ██    ██ ██   ██ ██   ██     ██      ██      ██   ██ ██ ██   ██    ██    **\n");
            Expressions::ReportInfo("** ██      ██    ██ ███████ ██   ██     ███████ ██      ██████  ██ ██████     ██    **\n");
            Expressions::ReportInfo("** ██      ██    ██ ██   ██ ██   ██          ██ ██      ██   ██ ██ ██         ██    **\n");
            Expressions::ReportInfo("** ███████  ██████  ██   ██ ██████      ███████  ██████ ██   ██ ██ ██         ██    **\n");
            Expressions::ReportInfo("**                                                                                  **\n");
            Expressions::ReportInfo("**************************************************************************************\n");
            Expressions::ReportInfo("**************************************************************************************\n");

            scriptBlocks[currentScriptIndex].Set(tokens);

            Expressions::ReportInfo("**************************************************************************************\n");
            Expressions::ReportInfo("**************************************************************************************\n");
            printf("\n\ntokens.currIndex(%d) of tokens.count(%d) reached end of 'script'\n\n", tokens.currIndex, tokens.count);
            Expressions::ReportInfo("**************************************************************************************\n");
            Expressions::ReportInfo("**************************************************************************************\n");
        }

        bool ScriptsBlock::LoadAllActiveScripts()
        {
            delete[] scriptBlocks;
            scriptBlocks = nullptr;
            scriptBlocksCount = 0;
            
            // here i will load the active scripts file and parse which scripts to load
            // and how many to load
            // currently for development first test we only load one file
            
            int count = 1; // set to 1 for development test
            scriptBlocks = new ScriptBlock[count];
            scriptBlocksCount = count;

            for (int i = 0;i<count;i++) {
                currentScriptIndex = i;
                // this should now pass and execute the given callback
                ScriptEngine::Parser::ReadAndParseScriptFile(DEMO_SCRIPT_FILE_PATH, ScriptFileParsed);
            }
            return true;
        }

        bool ValidateAndLoadAllActiveScripts()
        {
            ScriptsBlock::running = false;
            ScriptEngine::Expressions::CalcStackSizesInit();
            if (ValidateAllActiveScripts() == false) return false;
            
            ScriptEngine::Expressions::InitStacks();
            if (ScriptsBlock::LoadAllActiveScripts() == false) {
#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
#include <iostream>
                std::cout << "Serious problem could not load scripts\n";
#endif
                return false;
            }
            ScriptsBlock::running = true;
            return true;
        }
    }
}