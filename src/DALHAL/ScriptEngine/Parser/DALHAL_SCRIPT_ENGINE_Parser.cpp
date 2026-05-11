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

#include <Arduino.h> // Needed for String class
#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
#include <iostream> // including this take 209512 bytes flash

#endif
#include <string>

#include "DALHAL_SCRIPT_ENGINE_Parser.h"

#include "DALHAL_SCRIPT_ENGINE_Tokenizer.h"
#include "../DALHAL_SCRIPT_ENGINE_Reports.h"
#include "DALHAL_SCRIPT_ENGINE_Parser_Conditions.h"
#include "DALHAL_SCRIPT_ENGINE_Parser_Actions.h"

#if defined(ESP32) || defined(ESP8266)
#include <Support/LittleFS_ext.h>
#else
#include <LittleFS_ext.h>
#endif

#include <Support/MeasureTime.h>

namespace DALHAL {
    namespace ScriptEngine {
        namespace Parser {

            bool ValidateParseScript(ScriptTokens& _tokens, bool validateOnly) {

                if (validateOnly) {
                    ReportInfo(String(F("**********************************************************************************\n")).c_str());
                    ReportInfo(String(F("*                            RAW TOKEN LIST                                      *\n")).c_str());
                    ReportInfo(String(F("**********************************************************************************\n")).c_str());
    #if defined(_WIN32) || defined(__linux__) || defined(__APPLE__) || defined(DEBUG_PRINT_SCRIPT_ENGINE)
                    ReportInfo(PrintScriptTokens(_tokens,0) + "\n");
    #endif
                    ReportInfo(String(F("\n VerifyBlocks (BetterError): ")).c_str());
                    //MEASURE_TIME(" VerifyBlocks time: ",
                    if (validateOnly && Conditions::VerifyBlocks(_tokens) == false) {
                        ReportInfo(String(F("[FAIL]\n")).c_str());
                        return false;
                    }
                    //);
                    ReportInfo(String(F("[OK]\n")).c_str());
                }
                // if here then we can safely parse all blocks
                //MEASURE_TIME("\n MergeConditions time: ",
                ReportInfo(String(F("\n MergeConditions: ")).c_str());
                Conditions::MergeConditions(_tokens);
                ReportInfo(String(F("[OK]\n")).c_str());
                //);
                //MEASURE_TIME("\n MergeActions time: ",
                ReportInfo(String(F("\n MergeActions: ")).c_str());
                Actions::MergeActions(_tokens);
                ReportInfo(String(F("[OK]\n")).c_str());
                //);
                //MEASURE_TIME("\n CountBlockItems time: ",
                ReportInfo(String(F("\n CountBlockItems: ")).c_str());
                Conditions::CountBlockItems(_tokens); // sets the metadata itemsInBlock
                ReportInfo(String(F("[OK]\n")).c_str());
                //);
                ReportInfo(String(F("**********************************************************************************\n")).c_str());
                ReportInfo(String(F("*                            PARSED TOKEN LIST                                   *\n")).c_str());
                ReportInfo(String(F("**********************************************************************************\n")).c_str());
    #if (defined(_WIN32) || defined(__linux__) || defined(__APPLE__)) && defined(DEBUG_PRINT_SCRIPT_ENGINE) || defined(DEBUG_PRINT_SCRIPT_ENGINE)
                ReportInfo(PrintScriptTokens(_tokens,0) + "\n");
    #endif
                if (validateOnly) {
                    //MEASURE_TIME("\n Conditions::EnsureActionBlocksContainItems time: ",
                    ReportInfo(String(F("\n Conditions::EnsureBlocksContainItems: ")).c_str());
                    if (validateOnly && Conditions::EnsureBlocksContainItems(_tokens) == false) { // uses the metadata itemsInBlock to determine if there are invalid
                        ReportInfo(String(F("[FAIL]\n")).c_str());
                        return false;
                    }
                    ReportInfo(String(F("[OK]\n")).c_str());
                //);
                    
                // MEASURE_TIME("\n VerifyConditionBlocks time: ",
                    ReportInfo(String(F("\n VerifyConditionBlocks: \n")).c_str());
                    if (Conditions::VerifyConditionBlocks(_tokens) == false) {
                        ReportInfo(String(F("[FAIL @ VerifyConditionBlocks]\n")).c_str());
                        return false;
                    }
                //);
                //MEASURE_TIME("\n VerifyActionBlocks time: ",
                    ReportInfo(String(F("\n VerifyActionBlocks: \n")).c_str());
                    if (Actions::VerifyActionBlocks(_tokens) == false) {
                        ReportInfo(String(F("[FAIL @ VerifyActionBlocks]\n")).c_str());
                        return false;
                    }
                //);
                }
                return true;
            }

            bool ReadAndParseScriptFile(const char* filePath, void (*parsedOKcallback)(ScriptTokens& tokens)) {
                char* fileContents = nullptr;// = ReadFileToMutableBuffer(filePath, fileSize);
                MEASURE_TIME(String(F("ReadAndParseScriptFile - load_text_file time: ")).c_str(),
                LittleFS_ext::FileResult fileResult = LittleFS_ext::load_text_file(filePath, &fileContents);
                if (fileResult != LittleFS_ext::FileResult::Success) {
                    ReportInfo(String(F("Error: file could not be read/or is empty\n")).c_str());
                    return false;
                }
                );

                int tokenCount = 0;
            //MEASURE_TIME("CountTokens time: ",
                tokenCount = ParseAndTokenize<ScriptToken>(fileContents, nullptr, -1); // count in the same function
            //);
                ReportInfo(String(F("Token count: ")).c_str() + std::to_string(tokenCount) + "\n");
                ScriptTokens tokens(tokenCount);
                
                //MEASURE_TIME("Tokenize time: ",
                if (ParseAndTokenize(fileContents, tokens.items, tokenCount) == -1)
                {
                    ReportInfo(String(F("Error: could not Tokenize\n")).c_str());
                    delete[] fileContents;
                    return false;
                }
                //);
                bool anyError = false;
                MEASURE_TIME(String(F("ValidateParseScript time: ")).c_str(),
                if (ValidateParseScript(tokens, parsedOKcallback==nullptr)) {
                    ReportInfo(String(F("ParseScript [OK]\n")).c_str());
                    
                    if (parsedOKcallback) {
                        MEASURE_TIME(String(F("loadscript time: ")).c_str(),
                        parsedOKcallback(tokens);
                        );
                    }
                } else {
                    ReportInfo(String(F("ParseScript [FAIL]\n")).c_str());
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