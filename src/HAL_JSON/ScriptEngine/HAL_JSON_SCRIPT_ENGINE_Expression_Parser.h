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

#pragma once

#include <Arduino.h>
#include "../HAL_JSON_ZeroCopyString.h"
#include "../../../src/Support/Logger.h"
#include "../HAL_JSON_UID_Path.h"
#include "../HAL_JSON_Manager.h"
#include "HAL_JSON_SCRIPT_ENGINE_Parser_Token.h"
#include "HAL_JSON_SCRIPT_ENGINE_Expression_Token.h"
#include "HAL_JSON_SCRIPT_ENGINE_LogicExecNode.h"
#include "HAL_JSON_SCRIPT_ENGINE_Support.h"

#include <string>

#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
#include <iostream>
#endif

//#define HAL_JSON_SCRIPTS_EXPRESSIONS_PARSER_SHOW_DEBUG

#define HAL_JSON_SCRIPTS_EXPRESSIONS_MULTILINE_KEYWORD "\\"

#include <vector> // until we see it working

namespace HAL_JSON {
    namespace ScriptEngine {
        enum class ExpressionContext {
            IfCondition,
            Assignment
        };
        enum class ValidateOperandMode {
            Read,
            Write,
            ReadWrite
        };

        class Expressions {
        private:
            static const char* SingleOperatorList;
            static const char* DoubleOperatorList;

            static ExpressionTokens* rpnOutputStack;
            static int rpnOutputStackNeededSize;

            static ExpressionToken* opStack;
            static int opStackSizeNeededSize;
            static int opStackSize;

            /** used both for development test and as temporary structure for the final exec output */
            static LogicRPNNode* logicRPNNodeStackPool; // the fixed array o
            static LogicRPNNode** logicRPNNodeStack;// = new LogicRPNNode*[stackMaxSize]();
            
            /** development/release */
            static int finalOutputStackNeededSize;

        public:
            static void CalcStackSizesInit();
            static void CalcStackSizes(Tokens& tokens);
            static void PrintCalcedStackSizes();
            static void InitStacks();
            static void ClearStacks();

            static void ReportError(const char* msg, const char* param = nullptr);
            static void ReportWarning(const char* msg, const char* param = nullptr);
            static void ReportInfo(std::string msg);

            // Helper: returns true if c is a single-character operator
            static inline ExpTokenType IsSingleOp(char c) {
                if (c == '+') return ExpTokenType::CalcPlus;
                else if (c == '-') return ExpTokenType::CalcMinus;
                else if (c == '*')  return ExpTokenType::CalcMultiply;
                else if (c == '/') return ExpTokenType::CalcDivide;
                else if (c == '<') return ExpTokenType::CompareLessThan;
                else if (c == '>') return ExpTokenType::CompareGreaterThan;
                else if (c == '&') return ExpTokenType::CalcBitwiseAnd;
                else if (c == '|') return ExpTokenType::CalcBitwiseOr;
                else if (c == '^') return ExpTokenType::CalcBitwiseExOr;
                else if (c == '%') return ExpTokenType::CalcModulus;
                return ExpTokenType::NotSet;
            }

            // Helper: checks if c + next form a 2-char operator
            static inline ExpTokenType IsTwoCharOp(const char* c) {
                char first = *(c++);
                char next = *c;
                if (first == '&' && next == '&') return ExpTokenType::LogicalAnd;
                else if (first == '|' && next == '|') return ExpTokenType::LogicalOr;
                else if (first == '=' && next == '=') return ExpTokenType::CompareEqualsTo;
                else if (first == '!' && next == '=') return ExpTokenType::CompareNotEqualsTo;
                else if (first == '<' && next == '=') return ExpTokenType::CompareLessThanOrEqual;
                else if (first == '>' && next == '=') return ExpTokenType::CompareGreaterThanOrEqual;
                else if (first == '>' && next == '>') return ExpTokenType::CalcBitwiseRightShift;
                else if (first == '<' && next == '<') return ExpTokenType::CalcBitwiseLeftShift;
                return ExpTokenType::NotSet;
            }

            static inline bool IsSingleCompare(char c) {
                if (c == '<') return true;
                else if (c == '>') return true;
                return false;
            }

            static inline bool IsLogicOrCompare(const char* c) {
                char first = *(c++);
                char next = *c;
                if (first == '&' && next == '&') return true;
                else if (first == '|' && next == '|') return true;
                else if (first == '=' && next == '=') return true;
                else if (first == '!' && next == '=') return true;
                else if (first == '<' && next == '=') return true;
                else if (first == '>' && next == '=') return true;
                return false;
            }
            
            //static void GetOperands(Tokens& tokens, ZeroCopyString* operands, int operandCount);
            
            /** returns nullptr if no invalid char is found, otherwise it will return the character */
            
        public:
            static const char* ValidOperandVariableName(const Token& operand);
            static void CountOperatorsAndOperands(Tokens& tokens, int& operatorCount, int& operandCount, int& leftParenthesisCount, int& rightParenthesisCount, ExpressionContext exprContext);
            static void ValidateOperand(const Token& operand, bool& anyError, ValidateOperandMode mode = ValidateOperandMode::Read);
            //static bool OperandIsVariable(const Token& operand);
            static bool IsSingleOperator(char c);
            static bool IsDoubleOperator(const char* c);
            static bool IsValidOperandChar(char c);
            static bool ValidateExpression(Tokens& tokens, ExpressionContext exprContext);

            static std::string CalcExpressionToString(const LogicRPNNode* node);
            static std::string CalcExpressionToString(int startIndex, int endIndex);
            static void printLogicRPNNodeTree(const LogicRPNNode* node, int indent = 0);
            static void PrintLogicRPNNodeAdvancedTree(const LogicRPNNode* node, int depth = 0);

            static void GetGenerateRPNTokensCount_PreCalc(const Tokens& tokens, int& totalCount, int& operatorCount, int& finalOutputSize);
            static int GetGenerateRPNTokensCount_DryRun(const Tokens& tokens, int initialSize);
           
            /** the returned pointer here is owned and need to be deleted by the caller */
            static LogicRPNNode* BuildLogicTree(ExpressionTokens* tokens);
            /** special note here the returned pointer is non-owned and belongs to the global stack of Expressions */
            static ExpressionTokens* GenerateRPNTokens(Tokens& tokens);
        };
    }
}