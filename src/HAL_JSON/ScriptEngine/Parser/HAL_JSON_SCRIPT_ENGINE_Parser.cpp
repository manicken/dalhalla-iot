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

#include <Arduino.h> // Needed for String class
#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
#include <iostream> // including this take 209512 bytes flash

#endif
#include <string>

#include "HAL_JSON_SCRIPT_ENGINE_Parser.h"

#include "HAL_JSON_SCRIPT_ENGINE_Tokenizer.h"
#include "../HAL_JSON_SCRIPT_ENGINE_Reports.h"
#include "HAL_JSON_SCRIPT_ENGINE_Parser_Conditions.h"
#include "HAL_JSON_SCRIPT_ENGINE_Parser_Actions.h"

#if defined(ESP32) || defined(ESP8266)
#include "../../../Support/LittleFS_ext.h"
#else
#include <LittleFS_ext.h>
#endif

#include "../../../Support/MeasureTime.h"

namespace HAL_JSON {
    namespace ScriptEngine {
        namespace Parser {

            bool ValidateParseScript(ScriptTokens& _tokens, bool validateOnly) {

                if (validateOnly) {
                    ReportInfo("**********************************************************************************\n");
                    ReportInfo("*                            RAW TOKEN LIST                                      *\n");
                    ReportInfo("**********************************************************************************\n");
    #if defined(_WIN32) || defined(__linux__) || defined(__APPLE__) || defined(DEBUG_PRINT_SCRIPT_ENGINE)
                    ReportInfo(PrintScriptTokens(_tokens,0) + "\n");
    #endif
                    ReportInfo("\n VerifyBlocks (BetterError): ");
                    //MEASURE_TIME(" VerifyBlocks time: ",
                    if (validateOnly && Conditions::VerifyBlocks(_tokens) == false) {
                        ReportInfo("[FAIL]\n");
                        return false;
                    }
                    //);
                    ReportInfo("[OK]\n");
                }
                // if here then we can safely parse all blocks
                //MEASURE_TIME("\n MergeConditions time: ",
                ReportInfo("\n MergeConditions: ");
                Conditions::MergeConditions(_tokens);
                ReportInfo("[OK]\n");
                //);
                //MEASURE_TIME("\n MergeActions time: ",
                ReportInfo("\n MergeActions: ");
                Actions::MergeActions(_tokens);
                ReportInfo("[OK]\n");
                //);
                //MEASURE_TIME("\n CountBlockItems time: ",
                ReportInfo("\n CountBlockItems: ");
                Conditions::CountBlockItems(_tokens); // sets the metadata itemsInBlock
                ReportInfo("[OK]\n");
                //);
                ReportInfo("**********************************************************************************\n");
                ReportInfo("*                            PARSED TOKEN LIST                                   *\n");
                ReportInfo("**********************************************************************************\n");
    #if (defined(_WIN32) || defined(__linux__) || defined(__APPLE__)) && defined(DEBUG_PRINT_SCRIPT_ENGINE) || defined(DEBUG_PRINT_SCRIPT_ENGINE)
                ReportInfo(PrintScriptTokens(_tokens,0) + "\n");
    #endif
                if (validateOnly) {
                    //MEASURE_TIME("\n Conditions::EnsureActionBlocksContainItems time: ",
                    ReportInfo("\n Conditions::EnsureBlocksContainItems: ");
                    if (validateOnly && Conditions::EnsureBlocksContainItems(_tokens) == false) { // uses the metadata itemsInBlock to determine if there are invalid
                        ReportInfo("[FAIL]\n");
                        return false;
                    }
                    ReportInfo("[OK]\n");
                //);
                    
                // MEASURE_TIME("\n VerifyConditionBlocks time: ",
                    ReportInfo("\n VerifyConditionBlocks: \n");
                    if (Conditions::VerifyConditionBlocks(_tokens) == false) {
                        ReportInfo("[FAIL @ VerifyConditionBlocks]\n");
                        return false;
                    }
                //);
                //MEASURE_TIME("\n VerifyActionBlocks time: ",
                    ReportInfo("\n VerifyActionBlocks: \n");
                    if (Actions::VerifyActionBlocks(_tokens) == false) {
                        ReportInfo("[FAIL @ VerifyActionBlocks]\n");
                        return false;
                    }
                //);
                }
                return true;
            }

            bool ReadAndParseScriptFile(const char* filePath, void (*parsedOKcallback)(ScriptTokens& tokens)) {
                char* fileContents = nullptr;// = ReadFileToMutableBuffer(filePath, fileSize);
                MEASURE_TIME("ReadAndParseScriptFile - load_text_file time: ",
                LittleFS_ext::FileResult fileResult = LittleFS_ext::load_text_file(filePath, &fileContents);
                if (fileResult != LittleFS_ext::FileResult::Success) {
                    ReportInfo("Error: file could not be read/or is empty\n");
                    return false;
                }
                );

                int tokenCount = 0;
            //MEASURE_TIME("CountTokens time: ",
                tokenCount = ParseAndTokenize<ScriptToken>(fileContents, nullptr, -1); // count in the same function
            //);
                ReportInfo("Token count: " + std::to_string(tokenCount) + "\n");
                ScriptTokens tokens(tokenCount);
                
                //MEASURE_TIME("Tokenize time: ",
                if (ParseAndTokenize(fileContents, tokens.items, tokenCount) == -1)
                {
                    ReportInfo("Error: could not Tokenize\n");
                    delete[] fileContents;
                    return false;
                }
                //);
                bool anyError = false;
                MEASURE_TIME("ValidateParseScript time: ",
                if (ValidateParseScript(tokens, parsedOKcallback==nullptr)) {
                    ReportInfo("ParseScript [OK]\n");
                    
                    if (parsedOKcallback) {
                        MEASURE_TIME("loadscript time: ",
                        parsedOKcallback(tokens);
                        );
                    }
                } else {
                    ReportInfo("ParseScript [FAIL]\n");
                    anyError = true;
                }
                );
                // dont forget to free/delete
                delete[] fileContents;
                return anyError == false;
            }
        }
    }
}