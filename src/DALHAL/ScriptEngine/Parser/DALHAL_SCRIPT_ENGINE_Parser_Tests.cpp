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

#include "DALHAL_SCRIPT_ENGINE_Parser_Tests.h"

//#include "DALHAL_SCRIPT_ENGINE_Parser.h"
#include "DALHAL_SCRIPT_ENGINE_Tokenizer.h"
#include "DALHAL_SCRIPT_ENGINE_Parser_Actions.h"
#include "DALHAL_SCRIPT_ENGINE_Parser_Expressions.h"

#include "../DALHAL_SCRIPT_ENGINE_Reports.h"

#if defined(ESP32) || defined(ESP8266)
#include "../../../Support/LittleFS_ext.h"
#else
#include <LittleFS_ext.h>
#endif

#include "../../../Support/MeasureTime.h"

namespace DALHAL {
    namespace ScriptEngine {
        namespace Parser {
            namespace Tests {

                bool ParseExpressionTest(const char* filePath) {
                    
                    char* fileContents = nullptr;// = ReadFileToMutableBuffer(filePath, fileSize);
                    LittleFS_ext::FileResult fileResult = LittleFS_ext::load_text_file(filePath, &fileContents);
                    if (fileResult != LittleFS_ext::FileResult::Success) {
                        ReportInfo("Error: file could not be read/or is empty\n");
                        return false;
                    }

                    int tokenCount = ParseAndTokenize<ScriptToken>(fileContents);
                    ReportInfo("Token count: " + std::to_string(tokenCount) + "\n");
                    ScriptTokens tokens(tokenCount);
                    
                    MEASURE_TIME("Tokenize time: ",

                    if (ParseAndTokenize(fileContents, tokens.items, tokenCount) == false) {
                        ReportInfo("Error: could not Tokenize\n");
                        delete[] fileContents;
                        return false;
                    }
                    
                    );
                    
                    ReportInfo("**********************************************************************************\n");
                    ReportInfo("*                            PARSED TOKEN LIST                                   *\n");
                    ReportInfo("**********************************************************************************\n");

                    ReportInfo(PrintScriptTokens(tokens,0) + "\n");

                    ReportInfo("**********************************************************************************\n");
                    ReportInfo("*                            VALIDATE PARSED TOKEN LIST                          *\n");
                    ReportInfo("**********************************************************************************\n");
                    // need to be done before any ValidateExpression
                    // and that normally mean before any validation first occur
                    // i.e if many script files are to be validated this need to happen before any of that happens
                    Expressions::CalcStackSizesInit();
                    tokens.currentEndIndex = tokens.count;
                    tokens.firstTokenStartOffset = nullptr;
                    tokens.currIndex = 0;
                    if (false == Expressions::ValidateExpression(tokens))
                    {
                        ReportInfo("Error: validate tokens fail\n");
                        delete[] fileContents;
                        return false;
                    }
                    Expressions::PrintCalcedStackSizes();
                    Expressions::InitStacks();

                    ReportInfo("\nInput expression: " + tokens.ToString());

                    ExpressionTokens* newDirect = Expressions::GenerateRPNTokens(tokens);
                    LogicRPNNode* lrpnNode = Expressions::BuildLogicTree(newDirect);
                    ReportInfo("\n\nnew complete RPN:");
    #if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
                    for (int i=0;i<newDirect->currentCount;i++) { // currentCount is set by GenerateRPNTokens and defines the current 'size'
                        ExpressionToken& tok = newDirect->items[i];
                        //if (tok->type == TokenType::Operand)
                            ReportInfo(tok.ToString() + " ");
                        //else
                        //    ReportInfo(TokenTypeToString(tok->type ) + std::string(" "));
                    }
    #endif
                    ReportInfo("\n");
                    ReportInfo("\n\ntree view:\n");
    #if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
                    Expressions::printLogicRPNNodeTree(lrpnNode, 0);
                    //ReportInfo("\n\nadvanced tree view:\n");
                    //Expressions::PrintLogicRPNNodeAdvancedTree(lrpnNode, 0);
    #endif
                    Expressions::ClearStacks();
                    delete[] fileContents;

                    ReportInfo("\nAll done!!!\n");
                    return true;
                }

                bool ParseActionExpressionTest(const char* filePath) {
                    
                    char* fileContents = nullptr;
                    LittleFS_ext::FileResult fileResult = LittleFS_ext::load_text_file(filePath, &fileContents);
                    if (fileResult != LittleFS_ext::FileResult::Success) {
                        ReportInfo("Error: file could not be read/or is empty\n");
                        return false;
                    }

                    int tokenCount = ParseAndTokenize<ScriptToken>(fileContents);
                    ReportInfo("Token count: " + std::to_string(tokenCount) + "\n");
                    ScriptTokens tokens(tokenCount);
                    
                    MEASURE_TIME("Tokenize time: ",

                    if (ParseAndTokenize(fileContents, tokens.items, tokenCount) == false) {
                        ReportInfo("Error: could not Tokenize\n");
                        delete[] fileContents;
                        return false;
                    }
                    
                    );
                    
                    ReportInfo("**********************************************************************************\n");
                    ReportInfo("*                            PARSED TOKEN LIST                                   *\n");
                    ReportInfo("**********************************************************************************\n");

                    ReportInfo(PrintScriptTokens(tokens,0) + "\n");

                    ReportInfo("\nInput action expression: " + tokens.ToString() + "\n");

                    tokens.currIndex = 0;
                    tokens.items[0].itemsInBlock = tokens.count; // set as a block so that ExtractAssignmentParts can work as expected
                    if (false == DALHAL::ScriptEngine::Parser::Actions::ExtractAssignmentParts(tokens))
                    {
                        ReportInfo("Error: ExtractAssignmentParts fail\n");
                        delete[] fileContents;
                        return false;
                    }

                    ReportInfo("**********************************************************************************\n");
                    ReportInfo("*                            VALIDATE PARSED TOKEN LIST                          *\n");
                    ReportInfo("**********************************************************************************\n");
                    // need to be done before any ValidateExpression
                    // and that normally mean before any validation first occur
                    // i.e if many script files are to be validated this need to happen before any of that happens
                    Expressions::CalcStackSizesInit();
                    if (Expressions::ValidateExpression(DALHAL::ScriptEngine::Parser::Actions::AssignmentParts::rhs) == false)
                    {
                        ReportInfo("Error: validate tokens fail\n");
                        delete[] fileContents;
                        return false;
                    }
                    Expressions::PrintCalcedStackSizes();
                    Expressions::InitStacks();

                    ReportInfo("\nAction lhs:" + DALHAL::ScriptEngine::Parser::Actions::AssignmentParts::lhs.ToString() + "\n");
                    ReportInfo("Action assigment operator:" + std::string(1, DALHAL::ScriptEngine::Parser::Actions::AssignmentParts::op) + "\n\n");

                    ExpressionTokens* newDirect = Expressions::GenerateRPNTokens(DALHAL::ScriptEngine::Parser::Actions::AssignmentParts::rhs);
                    if (newDirect == nullptr) {
                        printf("\n!!!!!!!!!!!!!!!!!!!!!!!!!! ParseActionExpressionTest - newDirect was nullptr\n");
                        Expressions::ClearStacks();
                        delete[] fileContents;
                        return false;
                    }
                    
                    ReportInfo("\n\nAction rhs calc RPN:");
                    for (int i=0;i<newDirect->currentCount;i++) { // currentCount is set by GenerateRPNTokens and defines the current 'size'
                        ExpressionToken& tok = newDirect->items[i];
                        //if (tok->type == TokenType::Operand)
                            ReportInfo(tok.ToString() + " ");
                        //else
                        //    ReportInfo(TokenTypeToString(tok->type ) + std::string(" "));
                    }
                    ReportInfo("\n");
                    
                    Expressions::ClearStacks();
                    delete[] fileContents;

                    ReportInfo("\nAll done!!!\n");
                    return true;
                }
            }
        }
    }
}