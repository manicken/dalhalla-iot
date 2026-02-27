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
#include "../../Core/Types/DALHAL_ZeroCopyString.h"


#include "DALHAL_SCRIPT_ENGINE_Script_Token.h"
#include "DALHAL_SCRIPT_ENGINE_Expression_Token.h"
#include "../Runtime/DALHAL_SCRIPT_ENGINE_LogicExecNode.h"


#include <string>

#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
#include <iostream>
#endif

//#define DALHAL_SCRIPTS_EXPRESSIONS_PARSER_SHOW_DEBUG

#define DALHAL_SCRIPTS_EXPRESSIONS_MULTILINE_KEYWORD "\\"

#include <vector> // until we see it working

namespace DALHAL {
    namespace ScriptEngine {
        enum class ExpressionContext {
            IfCondition,
            Assignment
        };
        enum class ValidateOperandMode {
            UnSet,
            Read,
            Write,
            ReadWrite,
            Exec
        };

        enum class OperandTargetInfoResult {
            Success,
            OperandIsConst,
           // OperandInvalidCharacter,
            DeviceNotFound
            
        };

        struct OperandTargetInfo {
            ZeroCopyString funcName;
            /** used only on bracket [] operation statements */
            bool isBracketAccess = false;
            Device* device = nullptr;
        };

        class Expressions {
        private:
            static const char* SingleOperatorList;
            static const char* DoubleOperatorList;
            /** used when reading script */
            static ExpressionTokens* rpnOutputStack;
            /** used when reading script */
            static int rpnOutputStackNeededSize;

            /** used when reading script */
            static ExpressionToken* opStack;
            /** used when reading script */
            static int opStackSizeNeededSize;
            /** used when reading script */
            static int opStackSize;

            /** used both for development test and as temporary structure for the final exec output */
            static LogicRPNNode* logicRPNNodeStackPool; // using a array to minimize heap fragmentation
            static LogicRPNNode** logicRPNNodeStack;// = new LogicRPNNode*[stackMaxSize]();
            
            /** development/release */
            static int finalOutputStackNeededSize;

        public:
            static void CalcStackSizesInit();
            static void CalcStackSizes(ScriptTokens& tokens);
            static void PrintCalcedStackSizes();
            static void InitStacks();
            static void ClearStacks();

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
            static const char* ValidOperandVariableName(const ScriptToken& operand);
            static void ValidateStructure(ScriptTokens& tokens, bool& anyError);
            static OperandTargetInfoResult ParseOperandTarget(const ScriptToken& operandToken, bool& anyError, OperandTargetInfo& outInfo);
            static void ValidateOperand(const ScriptToken& operand, bool& anyError, ValidateOperandMode mode = ValidateOperandMode::Read);
            //static bool OperandIsVariable(const Token& operand);
            static bool IsSingleOperator(char c);
            static bool IsDoubleOperator(const char* c);
            static bool IsValidOperandChar(char c);
            static bool IsExpressionEmpty(const ScriptTokens& tokens);
            static bool ValidateExpression(ScriptTokens& tokens);

            static std::string CalcExpressionToString(const LogicRPNNode* node);
            static std::string CalcExpressionToString(int startIndex, int endIndex);
            static void printLogicRPNNodeTree(const LogicRPNNode* node, int indent = 0);
            static void PrintLogicRPNNodeAdvancedTree(const LogicRPNNode* node, int depth = 0);

            static void GetGenerateRPNTokensCount_PreCalc(const ScriptTokens& tokens, int& totalCount, int& operatorCount, int& finalOutputSize);
            static int GetGenerateRPNTokensCount_DryRun(const ScriptTokens& tokens, int initialSize);
           
            /** the returned pointer here is owned and need to be deleted by the caller */
            static LogicRPNNode* BuildLogicTree(ExpressionTokens* tokens);
            /** special note here the returned pointer is non-owned and belongs to the global stack of Expressions */
            static ExpressionTokens* GenerateRPNTokens(ScriptTokens& tokens);
        };
    }
}