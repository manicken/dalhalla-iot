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

#include "DALHAL_SCRIPT_ENGINE_ScriptsToLoad.h"

#include <DALHAL/ScriptEngine/Parser/DALHAL_SCRIPT_ENGINE_Tokenizer.h> // ParseAndTokenize
#include <DALHAL/ScriptEngine/DALHAL_SCRIPT_ENGINE_Reports.h>
#include <DALHAL/Support/DALHAL_Logger.h>

#include <LittleFS.h>

#if defined(ESP32) || defined(ESP8266)
#include <Support/LittleFS_ext.h>
#else
#include <LittleFS_ext.h>
#endif



namespace DALHAL {
    namespace ScriptEngine {

        ScriptsToLoad::ScriptsToLoad() : scriptsListContents(nullptr), scriptFileList(nullptr), scriptFileCount(0) {
            bool useDefaultFile = false;

            if (LittleFS.exists(DALHAL_SCRIPT_ENGINE_SCRIPTS_LIST_FILE_PATH)) {
                
                LittleFS_ext::FileResult res = LittleFS_ext::load_text_file(DALHAL_SCRIPT_ENGINE_SCRIPTS_LIST_FILE_PATH, &scriptsListContents);
                if (res != LittleFS_ext::FileResult::Success) {
                    useDefaultFile = true;
                }
                else {
                    //printf("\nUsing scripts list file:\n");

                    int scriptCount = ParseAndTokenize<ZeroCopyString>(scriptsListContents);
                    //printf("\nscript count:%d\n", scriptCount);
                    InitScriptList(scriptCount);
                    if (false == ParseAndTokenize(scriptsListContents, scriptFileList, scriptFileCount)) {
                        useDefaultFile = true;
                        GlobalLogger.Error(F("\ntokenize scripts list file fail!\n"));
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
                GlobalLogger.Info(F("Using default script file: script.txt"));
                InitScriptList(1);
                scriptFileList[0].Set(DALHAL_SCRIPT_ENGINE_DEFAULT_SCRIPT_FILE);
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
    }
}