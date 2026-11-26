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

#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
#include <iostream> // including this take 209512 bytes flash

#endif
#include <string>

#include "DALHAL_SCRIPT_ENGINE_Parser.h"

#include "DALHAL_SCRIPT_ENGINE_Tokenizer.h"
#include "../DALHAL_SCRIPT_ENGINE_Reports.h"
#include "DALHAL_SCRIPT_ENGINE_Parser_Triggers.h"
#include "DALHAL_SCRIPT_ENGINE_Parser_Conditions.h"
#include "DALHAL_SCRIPT_ENGINE_Parser_Actions.h"

#if defined(ESP32) || defined(ESP8266)
#include <Support/LittleFS_ext.h>
#else
#include <LittleFS_ext.h>
#endif

#include <Support/MeasureTime.h>

#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
#include <fstream>
#include <string>

void writeDebug(std::string fileName, const std::string& text)
{
    std::ofstream file(fileName); // truncates automatically
    if (file)
    {
        file << text;
    }
}
#endif

namespace DALHAL {
    namespace ScriptEngine {
        namespace Parser {

            bool ValidateParseScript(ScriptTokens& _tokens, bool validateOnly, const char* fileName) {
                bool anyError = false;
                if (validateOnly) {
                    ReportInfo(DALHAL::ToStdString(F("**********************************************************************************\n")));
                    ReportInfo(DALHAL::ToStdString(F("*                            RAW TOKEN LIST                                      *\n")));
                    ReportInfo(DALHAL::ToStdString(F("**********************************************************************************\n")));
#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
                    {
                    std::string debugFilename = fileName;
                    writeDebug(debugFilename + "_RawTokens.txt", PrintScriptTokens(_tokens,0));
                    }
#endif
                    ReportInfo(DALHAL::ToStdString(F("\n VerifyBlocks (BetterError): ")));
                    //MEASURE_TIME(" VerifyBlocks time: ",
                    if (validateOnly && Conditions::VerifyBlocks(_tokens) == false) {
                        ReportInfo(DALHAL::ToStdString(F("[FAIL]\n")));
                        anyError = true;
                        //return false;
                    }
                    //);
                    ReportInfo(DALHAL::ToStdString(F("[OK]\n")));
                }
                // if here then we can safely parse all blocks
                //MEASURE_TIME("\n MergeConditions time: ",
                ReportInfo(DALHAL::ToStdString(F("\n MergeConditions: ")));
                Conditions::MergeConditions(_tokens);
                ReportInfo(DALHAL::ToStdString(F("[OK]\n")));
                //);
                //MEASURE_TIME("\n MergeActions time: ",
                ReportInfo(DALHAL::ToStdString(F("\n MergeActions: ")));
                Actions::MergeActions(_tokens);
                ReportInfo(DALHAL::ToStdString(F("[OK]\n")));
                //);
                //MEASURE_TIME("\n CountBlockItems time: ",
                ReportInfo(DALHAL::ToStdString(F("\n CountBlockItems: ")));
                Conditions::CountBlockItems(_tokens); // sets the metadata itemsInBlock
                ReportInfo(DALHAL::ToStdString(F("[OK]\n")));
                //);
                ReportInfo(DALHAL::ToStdString(F("**********************************************************************************\n")));
                ReportInfo(DALHAL::ToStdString(F("*                            PARSED TOKEN LIST                                   *\n")));
                ReportInfo(DALHAL::ToStdString(F("**********************************************************************************\n")));

                if (validateOnly) {
                    //MEASURE_TIME("\n Conditions::EnsureActionBlocksContainItems time: ",
                    ReportInfo(DALHAL::ToStdString(F("\n Conditions::EnsureBlocksContainItems: ")));
                    if (Conditions::EnsureBlocksContainItems(_tokens) == false) { // uses the metadata itemsInBlock to determine if there are invalid
                        ReportInfo(DALHAL::ToStdString(F("[FAIL]\n")));
                        anyError = true;
                        //return false;
                    }
                    ReportInfo(DALHAL::ToStdString(F("[OK]\n")));
                //);
                    
                    ReportInfo(DALHAL::ToStdString(F("\n Trigger::Validate: ")));
                    if (Trigger::Validate(_tokens) == false) {
                        ReportInfo(DALHAL::ToStdString(F("[FAIL]\n")));
                        anyError = true;
                        //return false;
                    }
                    ReportInfo(DALHAL::ToStdString(F("[OK]\n")));
                    // TODO make functionality to check "script blocks"
                    // i.e. a script block is contained inside a on <event> then  <script> endon
                    // a script block is allways at the root
                    // but i do allow "if blocks" to be in the root as well
                    // then they are automatically wrapped inside a "script block" that is set to allway run
                    
                // MEASURE_TIME("\n VerifyConditionBlocks time: ",
                    ReportInfo(DALHAL::ToStdString(F("\n VerifyConditionBlocks: \n")));
                    if (Conditions::VerifyConditionBlocks(_tokens) == false) {
                        ReportInfo(DALHAL::ToStdString(F("[FAIL @ VerifyConditionBlocks]\n")));
                        anyError = true;
                        //return false;
                    }
                //);
                //MEASURE_TIME("\n VerifyActionBlocks time: ",
                    ReportInfo(DALHAL::ToStdString(F("\n VerifyActionBlocks: \n")));
                    if (Actions::VerifyActionBlocks(_tokens) == false) {
                        ReportInfo(DALHAL::ToStdString(F("[FAIL @ VerifyActionBlocks]\n")));
                        anyError = true;
                        //return false;
                    }
#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
                    {
                    std::string debugFilename = fileName;
                    writeDebug(debugFilename + "_ParsedTokens.txt", PrintScriptTokens(_tokens,0));
                    }
#endif

                //);
                }
                
                return anyError == false;
            }

            bool ReadAndParseScriptFile(const char* filePath, bool (*parsedOKcallback)(ScriptTokens& tokens)) {
                char* fileContents = nullptr;// = ReadFileToMutableBuffer(filePath, fileSize);
                MEASURE_TIME(DALHAL::ToStdString(F("ReadAndParseScriptFile - load_text_file time: ")).c_str(),
                LittleFS_ext::FileResult fileResult = LittleFS_ext::load_text_file(filePath, &fileContents);
                if (fileResult != LittleFS_ext::FileResult::Success) {
                    ReportInfo(DALHAL::ToStdString(F("Error: file could not be read/or is empty\n")));
                    return false;
                }
                );

                int tokenCount = 0;
            //MEASURE_TIME("CountTokens time: ",
                tokenCount = ParseAndTokenize<ScriptToken>(fileContents, nullptr, -1); // count in the same function
            //);
                ReportInfo(DALHAL::ToStdString(F("Token count: ")).c_str() + std::to_string(tokenCount) + "\n");
                ScriptTokens tokens(tokenCount);
                
                //MEASURE_TIME("Tokenize time: ",
                if (ParseAndTokenize(fileContents, tokens.items, tokenCount) == -1)
                {
                    ReportInfo(DALHAL::ToStdString(F("Error: could not Tokenize\n")));
                    delete[] fileContents;
                    return false;
                }
                //);
                bool anyError = false;
                MEASURE_TIME(DALHAL::ToStdString(F("ValidateParseScript time: ")).c_str(),
                if (ValidateParseScript(tokens, parsedOKcallback==nullptr, filePath)) {
                    ReportInfo(DALHAL::ToStdString(F("ParseScript [OK]\n")));
                    
                    if (parsedOKcallback) {
                        MEASURE_TIME(DALHAL::ToStdString(F("loadscript time: ")).c_str(),
                        if (parsedOKcallback(tokens)==false) {
                            anyError=true;
                        }
                        );
                    }
                } else {
                    ReportInfo(DALHAL::ToStdString(F("ParseScript [FAIL]\n")));
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