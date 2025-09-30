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
#define SCRIPTS_DIRECTORY   "scripts/"
#define DEFAULT_SCRIPT_FILE "script.txt"
#define SCRIPTS_LIST_FILE_PATH SCRIPTS_DIRECTORY "list.txt"
#else
#define SCRIPTS_DIRECTORY     "/scripts/"
#define DEFAULT_SCRIPT_FILE "script.txt"
#define SCRIPTS_LIST_FILE_PATH SCRIPTS_DIRECTORY "list.txt"
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

        ScriptsToLoad::ScriptsToLoad() : scriptsListContents(nullptr), scriptFileList(nullptr), scriptFileCount(0) {
            bool useDefaultFile = false;

            if (LittleFS.exists(SCRIPTS_LIST_FILE_PATH)) {
                
                LittleFS_ext::FileResult res = LittleFS_ext::load_text_file(SCRIPTS_LIST_FILE_PATH, &scriptsListContents);
                if (res != LittleFS_ext::FileResult::Success) {
                    useDefaultFile = true;
                }
                else {
                    printf("\nUsing scripts list file:\n");
                    Parser::FixNewLines(scriptsListContents);
                    Parser::StripComments(scriptsListContents);
                    int scriptCount = Parser::CountTokens(scriptsListContents);
                    printf("\nscript count:%d\n", scriptCount);
                    InitScriptList(scriptCount);
                    if (false == Parser::Tokenize(scriptsListContents, scriptFileList, scriptFileCount)) {
                        useDefaultFile = true;
                        printf("\ntokenize scripts list file fail!\n");
                    } else {
                        //for (int i=0;i<scriptCount;i++) {
                        //    std::string s = scriptsToLoad.scriptFileList[i].ToString();
                        //    printf("\nScript file:%s\n", s.c_str());
                        //}
                       
                    }
                    
                }
            } else {
                useDefaultFile = true;
            }
            if (useDefaultFile) {
                printf("\nUsing default script file: script.txt\n");
                InitScriptList(1);
                scriptFileList[0].Set(DEFAULT_SCRIPT_FILE);
            }
        }
        ScriptsToLoad::~ScriptsToLoad() {
            if (scriptsListContents != nullptr) {
                delete[] scriptsListContents;
                scriptsListContents = nullptr;
            }
            if (scriptFileList != nullptr) {
                delete[] scriptFileList;
                scriptFileList = nullptr;
            }
            scriptFileCount = 0;
        }
        void ScriptsToLoad::InitScriptList(int count) {
            if (scriptFileList != nullptr) {
                delete[] scriptFileList;
            }
            scriptFileList = nullptr;
            if (count > 0) {
                scriptFileList = new ZeroCopyString[count];
            }
            scriptFileCount = count;
        }

        bool ValidateAllActiveScripts(ScriptsToLoad& scriptsToLoad)
        {
            bool valid = true;
            int count = scriptsToLoad.scriptFileCount;
            ZeroCopyString* files = scriptsToLoad.scriptFileList;
            for (int i = 0;i<count;i++) {
                
                std::string path = SCRIPTS_DIRECTORY + files[i].ToString();
                printf("\nValidating script:%s\n",path.c_str());
                valid = ScriptEngine::Parser::ReadAndParseScriptFile(path.c_str(), nullptr);
                if (valid == false) return false;
            }
            return true;
        }

        bool ScriptsBlock::LoadAllActiveScripts(ScriptsToLoad& scriptsToLoad)
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
                std::string path = SCRIPTS_DIRECTORY + files[i].ToString();
                valid = ScriptEngine::Parser::ReadAndParseScriptFile(path.c_str(), ScriptFileParsed);
                if (valid == false) return false;
            }
            return true;
        }

        bool ValidateAndLoadAllActiveScripts()
        {
            ScriptsToLoad scriptsToLoad; // Automatically loads the scripts list file (or defaults to script.txt) on construction
            
            ScriptsBlock::running = false;
            ScriptEngine::Expressions::CalcStackSizesInit();
            if (ValidateAllActiveScripts(scriptsToLoad) == false) { 
                printf("\nValidateAllActiveScripts fail!\n");
                GlobalLogger.printAllLogs(Serial, false);
                return false;
            }
            
            ScriptEngine::Expressions::InitStacks();
            if (ScriptsBlock::LoadAllActiveScripts(scriptsToLoad) == false) {
                printf("\nSERIOUS problem could not load scripts!\"n");
                return false;
            }
            ScriptsBlock::running = true;
            return true;
        }
    }
}