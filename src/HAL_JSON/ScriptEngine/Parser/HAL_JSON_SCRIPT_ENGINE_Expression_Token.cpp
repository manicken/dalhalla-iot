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

#include "HAL_JSON_SCRIPT_ENGINE_Expression_Token.h"

#include "../../Support/HAL_JSON_Logger.h"

namespace HAL_JSON {
    namespace ScriptEngine {
        bool EqualsAny(ExpTokenType type, const ExpTokenType* candidates) {
            while(*candidates != ExpTokenType::NotSet) {
                if (type == *candidates) return true;
                candidates++;
            }
            return false;
        }
        const char* ExpTokenTypeToString(ExpTokenType type) {
            switch (type) {
                case ExpTokenType::NotSet: return "NotSet";
                case ExpTokenType::LeftParenthesis: return "LeftPar";
                case ExpTokenType::RightParenthesis: return "RightPar";
                case ExpTokenType::LogicalAnd: return "LogicalAnd";
                case ExpTokenType::LogicalOr: return "LogicalOr";
                case ExpTokenType::CompareEqualsTo: return "CompEq";
                case ExpTokenType::CompareNotEqualsTo: return "CompNotEq";
                case ExpTokenType::CompareLessThan: return "CompLess";
                case ExpTokenType::CompareGreaterThan: return "CompGreat";
                case ExpTokenType::CompareLessThanOrEqual: return "CompLessEq";
                case ExpTokenType::CompareGreaterThanOrEqual: return "CompGreatEq";
                case ExpTokenType::CalcPlus: return "CalcPlus";
                case ExpTokenType::CalcMinus: return "CalcMinus";
                case ExpTokenType::CalcMultiply: return "CalcMultiply";
                case ExpTokenType::CalcDivide: return "CalcDivide";
                case ExpTokenType::CalcModulus: return "CalcModulus";
                case ExpTokenType::CalcBitwiseAnd: return "BitwiseAnd";
                case ExpTokenType::CalcBitwiseOr: return "BitwiseOr";
                case ExpTokenType::CalcBitwiseExOr: return "BitwiseExOr";
                case ExpTokenType::CalcBitwiseLeftShift: return "LeftShift";
                case ExpTokenType::CalcBitwiseRightShift: return "RightShift";
                case ExpTokenType::VarOperand: return "VarOperand";
                case ExpTokenType::ConstValOperand: return "ConstValOperand";
                default: return "Unknown";
            }
        }

        ExpressionToken::ExpressionToken() : type(ExpTokenType::NotSet) {
            start = nullptr;
            end = nullptr;
        }
        ExpressionToken::ExpressionToken(const char* _start, const char* _end, ExpTokenType _type) {
            start = _start;
            end = _end;
            type = _type;
        }
        ExpressionToken::ExpressionToken(const char* _start, int length, ExpTokenType _type) {
            start = _start;
            end = _start + length;
            type = _type;
        }
        ExpressionToken::~ExpressionToken() {
            // nothing to free here
        }
        bool ExpressionToken::AnyType(const ExpTokenType* candidates) {
            while(*candidates != ExpTokenType::NotSet) {
                if (type == *candidates) return true;
                candidates++;
            }
            return false;
        }
        void ExpressionToken::Set(const char* _start, int length, ExpTokenType _type) {
            //printf("ptr, int overload: %c\n", *_start);
            start = _start;
            end = _start + length;
            type = _type;
        }
        void ExpressionToken::Set(const char* _start, const char* _end, ExpTokenType _type) {
            //printf("ptr, ptr overload: %c\n", *_start);
            start = _start;
            end = _end;
            type = _type;
        }

        ExpressionTokens::ExpressionTokens() {
            items = nullptr;
            count = 0;
            currentCount = 0;
            containLogicOperators = false;
        }
        ExpressionTokens::ExpressionTokens(int _count) {
            count = _count;
            items = new ExpressionToken[_count];
            containLogicOperators = false;
            currentCount = 0;
        }
        ExpressionTokens::~ExpressionTokens() {
            delete[] items; // this is allways owned
            items = nullptr;
        }
        
        std::string PrintExpressionTokens(ExpressionTokens& _tokens, int start, int end) {
            ExpressionToken* tokens = _tokens.items;
            int tokenCount = _tokens.count;
            std::string msg;
            if (end == -1) end = tokenCount;
 
            for (int i=start;i<end;i++) {
                ExpressionToken& tok = tokens[i];
                std::string msgLine = "Token[" + std::to_string(i) + "]  ";
                if (tok.type != ExpTokenType::ConstValOperand && tok.type != ExpTokenType::VarOperand) {
                    msgLine += "---";
                    msgLine += ExpTokenTypeToString(tok.type);
                    msgLine += "---";
                } else {
                    msgLine += tok.ToString();
                }
                msg += msgLine + "\n";
            }
            return msg;
        }

        std::string PrintExpressionTokensOneRow(ExpressionTokens& _tokens, int start, int end) {
            ExpressionToken* tokens = _tokens.items;
            int tokenCount = _tokens.count;
            std::string msg;
            if (end == -1) end = tokenCount;
 
            for (int i=start;i<end;i++) {
                ExpressionToken& tok = tokens[i];
                
                std::string msgLine;/* = "Token[" + std::to_string(i) + "]  ";*/
                /*if (tok.type != ExpTokenType::ConstValOperand && tok.type != ExpTokenType::VarOperand) {
                    msgLine += "-";
                    msgLine += ExpTokenTypeToString(tok.type);
                    msgLine += "-";
                } else {*/
                    msgLine += tok.ToString();
                //}
                msg += msgLine + " ";
            }
            return "[ " + msg + " ]";
        }
    }
}