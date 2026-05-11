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

#include "DALHAL_SCRIPT_ENGINE_Parser_Tests.h"

//#include "DALHAL_SCRIPT_ENGINE_Parser.h"
#include "DALHAL_SCRIPT_ENGINE_Tokenizer.h"
#include "DALHAL_SCRIPT_ENGINE_Parser_Actions.h"
#include "DALHAL_SCRIPT_ENGINE_Parser_Expressions.h"

#include "../DALHAL_SCRIPT_ENGINE_Reports.h"

#if defined(ESP32) || defined(ESP8266)
#include <Support/LittleFS_ext.h>
#else
#include <LittleFS_ext.h>
#endif

#include <Support/MeasureTime.h>

namespace DALHAL {
    namespace ScriptEngine {
        namespace Parser {
            namespace Tests {

                bool ParseExpressionTest(const char* filePath) {
                    
                    char* fileContents = nullptr;// = ReadFileToMutableBuffer(filePath, fileSize);
                    LittleFS_ext::FileResult fileResult = LittleFS_ext::load_text_file(filePath, &fileContents);
                    if (fileResult != LittleFS_ext::FileResult::Success) {
                        ReportInfo(String(F("Error: file could not be read/or is empty\n")).c_str());
                        return false;
                    }

                    int tokenCount = ParseAndTokenize<ScriptToken>(fileContents);
                    ReportInfo(String(F("Token count: ")).c_str() + std::to_string(tokenCount) + "\n");
                    ScriptTokens tokens(tokenCount);
                    
                    MEASURE_TIME(String(F("Tokenize time: ")).c_str(),

                    if (ParseAndTokenize(fileContents, tokens.items, tokenCount) == false) {
                        ReportInfo(String(F("Error: could not Tokenize\n")).c_str());
                        delete[] fileContents;
                        return false;
                    }
                    
                    );
                    
                    ReportInfo(String(F("**********************************************************************************\n")).c_str());
                    ReportInfo(String(F("*                            PARSED TOKEN LIST                                   *\n")).c_str());
                    ReportInfo(String(F("**********************************************************************************\n")).c_str());

                    ReportInfo(PrintScriptTokens(tokens,0) + "\n");

                    ReportInfo(String(F("**********************************************************************************\n")).c_str());
                    ReportInfo(String(F("*                            VALIDATE PARSED TOKEN LIST                          *\n")).c_str());
                    ReportInfo(String(F("**********************************************************************************\n")).c_str());
                    // need to be done before any ValidateExpression
                    // and that normally mean before any validation first occur
                    // i.e if many script files are to be validated this need to happen before any of that happens
                    Expressions::CalcStackSizesInit();
                    tokens.currentEndIndex = tokens.count;
                    tokens.firstTokenStartOffset = nullptr;
                    tokens.currIndex = 0;
                    if (false == Expressions::ValidateExpression(tokens))
                    {
                        ReportInfo(String(F("Error: validate tokens fail\n")).c_str());
                        delete[] fileContents;
                        return false;
                    }
                    Expressions::PrintCalcedStackSizes();
                    Expressions::InitStacks();

                    ReportInfo(String(F("\nInput expression: ")).c_str() + tokens.ToString());

                    ExpressionTokens* newDirect = Expressions::GenerateRPNTokens(tokens);
                    LogicRPNNode* lrpnNode = Expressions::BuildLogicTree(newDirect);
                    ReportInfo(String(F("\n\nnew complete RPN:")).c_str());
    #if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
                    for (int i=0;i<newDirect->currentCount;i++) { // currentCount is set by GenerateRPNTokens and defines the current 'size'
                        //ExpressionToken& tok = newDirect->items[i];
                        //if (tok->type == TokenType::Operand)
                            ReportInfo(newDirect->items[i].ToString() + " ");
                        //else
                        //    ReportInfo(TokenTypeToString(tok->type ) + std::string(" "));
                    }
    #endif
                    ReportInfo("\n");
                    ReportInfo(String(F("\n\ntree view:\n")).c_str());
    #if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
                    Expressions::printLogicRPNNodeTree(lrpnNode, 0);
                    //ReportInfo("\n\nadvanced tree view:\n");
                    //Expressions::PrintLogicRPNNodeAdvancedTree(lrpnNode, 0);
    #endif
                    Expressions::ClearStacks();
                    delete[] fileContents;

                    ReportInfo(String(F("\nAll done!!!\n")).c_str());
                    return true;
                }

                bool ParseActionExpressionTest(const char* filePath) {
                    
                    char* fileContents = nullptr;
                    LittleFS_ext::FileResult fileResult = LittleFS_ext::load_text_file(filePath, &fileContents);
                    if (fileResult != LittleFS_ext::FileResult::Success) {
                        ReportInfo(String(F("Error: file could not be read/or is empty\n")).c_str());
                        return false;
                    }

                    int tokenCount = ParseAndTokenize<ScriptToken>(fileContents);
                    ReportInfo(String(F("Token count: ")).c_str() + std::to_string(tokenCount) + "\n");
                    ScriptTokens tokens(tokenCount);
                    
                    MEASURE_TIME(String(F("Tokenize time: ")).c_str(),

                    if (ParseAndTokenize(fileContents, tokens.items, tokenCount) == false) {
                        ReportInfo(String(F("Error: could not Tokenize\n")).c_str());
                        delete[] fileContents;
                        return false;
                    }
                    
                    );
                    
                    ReportInfo(String(F("**********************************************************************************\n")).c_str());
                    ReportInfo(String(F("*                            PARSED TOKEN LIST                                   *\n")).c_str());
                    ReportInfo(String(F("**********************************************************************************\n")).c_str());

                    ReportInfo(PrintScriptTokens(tokens,0) + "\n");

                    ReportInfo(String(F("\nInput action expression: ")).c_str() + tokens.ToString() + "\n");

                    tokens.currIndex = 0;
                    tokens.items[0].itemsInBlock = tokens.count; // set as a block so that ExtractAssignmentParts can work as expected
                    if (false == DALHAL::ScriptEngine::Parser::Actions::ExtractAssignmentParts(tokens))
                    {
                        ReportInfo(String(F("Error: ExtractAssignmentParts fail\n")).c_str());
                        delete[] fileContents;
                        return false;
                    }

                    ReportInfo(String(F("**********************************************************************************\n")).c_str());
                    ReportInfo(String(F("*                            VALIDATE PARSED TOKEN LIST                          *\n")).c_str());
                    ReportInfo(String(F("**********************************************************************************\n")).c_str());
                    // need to be done before any ValidateExpression
                    // and that normally mean before any validation first occur
                    // i.e if many script files are to be validated this need to happen before any of that happens
                    Expressions::CalcStackSizesInit();
                    if (Expressions::ValidateExpression(DALHAL::ScriptEngine::Parser::Actions::AssignmentParts::rhs) == false)
                    {
                        ReportInfo(String(F("Error: validate tokens fail\n")).c_str());
                        delete[] fileContents;
                        return false;
                    }
                    Expressions::PrintCalcedStackSizes();
                    Expressions::InitStacks();

                    ReportInfo(String(F("\nAction lhs:")).c_str() + DALHAL::ScriptEngine::Parser::Actions::AssignmentParts::lhs.ToString() + "\n");
                    ReportInfo(String(F("Action assigment operator:")).c_str() + std::string(1, DALHAL::ScriptEngine::Parser::Actions::AssignmentParts::op) + "\n\n");

                    ExpressionTokens* newDirect = Expressions::GenerateRPNTokens(DALHAL::ScriptEngine::Parser::Actions::AssignmentParts::rhs);
                    if (newDirect == nullptr) {
                        printf(String(F("\n!!!!!!!!!!!!!!!!!!!!!!!!!! ParseActionExpressionTest - newDirect was nullptr\n")).c_str());
                        Expressions::ClearStacks();
                        delete[] fileContents;
                        return false;
                    }
                    
                    ReportInfo(String(F("\n\nAction rhs calc RPN:")).c_str());
                    for (int i=0;i<newDirect->currentCount;i++) { // currentCount is set by GenerateRPNTokens and defines the current 'size'
                        //ExpressionToken& tok = newDirect->items[i];
                        //if (tok->type == TokenType::Operand)
                            ReportInfo(newDirect->items[i].ToString() + " ");
                        //else
                        //    ReportInfo(TokenTypeToString(tok->type ) + std::string(" "));
                    }
                    ReportInfo("\n");
                    
                    Expressions::ClearStacks();
                    delete[] fileContents;

                    ReportInfo(String(F("\nAll done!!!\n")).c_str());
                    return true;
                }
            }
        }
    }
}