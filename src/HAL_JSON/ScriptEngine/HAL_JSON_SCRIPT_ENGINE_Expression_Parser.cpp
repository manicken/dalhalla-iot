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

#include "HAL_JSON_SCRIPT_ENGINE_Expression_Parser.h"
#include <unordered_map>
#include <cctype>  // for isspace, isdigit, isalpha

namespace HAL_JSON {
    namespace ScriptEngine {
        

        void Expressions::ReportError(const char* msg, const char* param) {
#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
            std::cout << "Error: " << msg << " " << ((param!=nullptr)?param:"") << std::endl;
#else
            GlobalLogger.Error(F("Expr Rule Parse:"), msg);
#endif
        }

        void Expressions::ReportWarning(const char* msg, const char* param) {
#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
            std::cout << "Warning: " << msg << " " << ((param!=nullptr)?param:"") << std::endl;
#else
            GlobalLogger.Warn(F("Expr Rule Parse:"), msg);
#endif
        }

        void Expressions::ReportInfo(std::string msg) {
#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
            std::cout << msg;
#else
            //GlobalLogger.Info(F("Expr Rule Parse:"), msg);
#endif
        }

        static const ExpTokenType compareOperators[] = {ExpTokenType::CompareEqualsTo, ExpTokenType::CompareNotEqualsTo,
                                                     ExpTokenType::CompareGreaterThanOrEqual, ExpTokenType::CompareLessThanOrEqual,
                                                     ExpTokenType::CompareGreaterThan, ExpTokenType::CompareLessThan, 
                                                     ExpTokenType::NotSet};

        ExpressionTokens* Expressions::rpnOutputStack = nullptr;
        int Expressions::rpnOutputStackNeededSize = 0;
        ExpressionToken* Expressions::opStack = nullptr;
        int Expressions::opStackSizeNeededSize = 0;
        
        LogicRPNNode* Expressions::logicRPNNodeStackPool = nullptr;
        LogicRPNNode** Expressions::logicRPNNodeStack = nullptr;
        /** development test only */
        int Expressions::finalOutputStackNeededSize = 0;

        void Expressions::CalcStackSizesInit() {
            rpnOutputStackNeededSize = 0;
            opStackSizeNeededSize = 0;
            /** development test only */
            finalOutputStackNeededSize = 0;
        }
        void Expressions::CalcStackSizes(Tokens& tokens) {
            int totalCount = 0;
            int operatorCount = 0;
            int finalOutputCount = 0;
            GetGenerateRPNTokensCount_PreCalc(tokens, totalCount, operatorCount, finalOutputCount);
            // Second pass: simulate the operator stack precisely
            operatorCount = GetGenerateRPNTokensCount_DryRun(tokens, operatorCount);

            if (operatorCount > opStackSizeNeededSize) opStackSizeNeededSize = operatorCount;
            if (totalCount > rpnOutputStackNeededSize) rpnOutputStackNeededSize = totalCount;
            if (finalOutputCount > finalOutputStackNeededSize) finalOutputStackNeededSize = finalOutputCount;
        }
        void Expressions::PrintCalcedStackSizes() {
            printf("\nrpnOutputStack NeededSize:%d\n", rpnOutputStackNeededSize);
            printf("opStackSize NeededSize:%d\n", opStackSizeNeededSize);
            /** development test only */
            printf("logicRPNNodeStack NeededSize:%d\n\n", finalOutputStackNeededSize);
        }
        void Expressions::InitStacks() {
            printf("\n*************************************************** InitStacks ********************************\n");
            int extraStackSize = 10;
            rpnOutputStackNeededSize += extraStackSize;
            opStackSizeNeededSize += extraStackSize;
            finalOutputStackNeededSize += extraStackSize;

            Expressions::PrintCalcedStackSizes();
            ClearStacks();

            rpnOutputStack = new ExpressionTokens(rpnOutputStackNeededSize);
            opStack = new ExpressionToken[opStackSizeNeededSize];
            logicRPNNodeStackPool = new LogicRPNNode[finalOutputStackNeededSize];
            logicRPNNodeStack = new LogicRPNNode*[finalOutputStackNeededSize];
            halValueStack.Init(rpnOutputStackNeededSize);
            printf("\n[DONE]\n");
        }
        void Expressions::ClearStacks() {
            
            delete rpnOutputStack;
            delete[] opStack;
            delete[] logicRPNNodeStack; // delete the ptr array first
            delete[] logicRPNNodeStackPool;            
        }
        // precedence map
        static const std::unordered_map<ExpTokenType, int> precedence = {
            // calc
            {ExpTokenType::CalcMultiply, 8}, {ExpTokenType::CalcDivide, 8}, {ExpTokenType::CalcModulus, 8},
            {ExpTokenType::CalcPlus, 7}, {ExpTokenType::CalcMinus, 7},
            {ExpTokenType::CalcBitwiseLeftShift, 6}, {ExpTokenType::CalcBitwiseRightShift, 6},
            {ExpTokenType::CalcBitwiseAnd, 5}, {ExpTokenType::CalcBitwiseExOr, 5}, {ExpTokenType::CalcBitwiseOr, 5},

            // compare
            {ExpTokenType::CompareGreaterThan, 4}, {ExpTokenType::CompareLessThan, 4},
            {ExpTokenType::CompareGreaterThanOrEqual, 4}, {ExpTokenType::CompareLessThanOrEqual, 4},
            {ExpTokenType::CompareEqualsTo, 3}, {ExpTokenType::CompareNotEqualsTo, 3},

            // logic
            {ExpTokenType::LogicalAnd, 2}, {ExpTokenType::LogicalOr, 1}

            // assigment

        };

        inline int getPrecedence(ExpTokenType t) {
            auto it = precedence.find(t);
            return (it != precedence.end()) ? it->second : -1;
        }

        const char* Expressions::SingleOperatorList = "+-*/%|&^><";
        const char* Expressions::DoubleOperatorList = "&&" "||" "==" "!=" ">=" "<=" "<<" ">>";

        bool Expressions::IsSingleOperator(char c) {
            const char* op = SingleOperatorList;
            while (*op != '\0') {
                if (*op == c) return true;
                ++op;
            }
            return false;
        }

        bool Expressions::IsDoubleOperator(const char* c) {
            if (c == nullptr) {
                ReportError("IsDoubleOperator c - was nullptr");
                return false;
            } else if (*c == '\n') {
                ReportError("IsDoubleOperator *c - was empty str");
                return false;
            } else if (*(c+1) == '\n') {
                ReportError("IsDoubleOperator *(c+1) - was empty str");
                return false;
            }
            //if (c == nullptr /*|| *c != '\0' */|| *(c + 1) == '\0') return false; // safety

            const char* op = DoubleOperatorList;
            while (*op != '\0') {
                if ((*op == *c) && (*(op+1) == *(c+1))) return true;
                op+=2;
            }
            return false;
        }

        bool Expressions::IsValidOperandChar(char c) {
            return 
                (c >= 'a' && c <= 'z') ||
                (c >= 'A' && c <= 'Z') ||
                (c >= '0' && c <= '9') ||
                c == '_' || 
                c == ':' || 
                c == '.' || 
                c == ',' || 
                c == '#';
        }

        void Expressions::CountOperatorsAndOperands(Tokens& tokens, int& operatorCount, int& operandCount, int& leftParenthesisCount, int& rightParenthesisCount, ExpressionContext exprContext) {
            operatorCount = 0;
            operandCount = 0;
            leftParenthesisCount  = 0;
            rightParenthesisCount = 0;
  
            bool inOperand = false;

            for (int cti=0;cti<tokens.count;cti++) { // cti = currTokenIndex
                Token& token = tokens.items[cti];
                const char* effectiveStart = nullptr;
                if (cti == 0 && tokens.firstTokenStartOffset != nullptr) {
                    effectiveStart  = tokens.firstTokenStartOffset;
                    //std::cout << "firstTokenStartOffset was true\n"; 
                } else {
                    effectiveStart  = token.start;
                }
                const char* tokenEnd = token.end;
                for (const char* p = effectiveStart; p < tokenEnd; p++) {
                    if (IsDoubleOperator(p)/* && exprContext == ExpressionContext::IfCondition*/) {
                        p++;
                        operatorCount++;
                        inOperand = false;
                    }
                    else if (IsSingleOperator(*p)) {
                        operatorCount++;
                        inOperand = false;
                    } else if (*p == '(') {
                        leftParenthesisCount++;
                        inOperand = false;
                    } else if (*p == ')') {
                        rightParenthesisCount++;
                        inOperand = false;
                    } else if (!inOperand) {
                        operandCount++;
                        inOperand = true;
                    }
                }
            }
        }
    
/* in favor of ZeroCopyString ValidNumber function
        bool Expressions::OperandIsVariable(const Token& operandToken) {
            const char* p = operandToken.start;
            const char* const end = operandToken.end;
            while (p < end) {
                if (isdigit(*p) == false) return true;
                p++;
            }
            return false;
        }
*/
        const char* Expressions::ValidOperandVariableName(const Token& operandToken) {
            const char* p = operandToken.start;
            const char* const end = operandToken.end;
            while (p < end) {
                if (IsValidOperandChar(*p) == false) {
                    return p;
                }
                p++;
            }
            return nullptr;
        }

        bool Expressions::ValidateExpression(Tokens& tokens, ExpressionContext exprContext) {
            
            bool anyError = false;

            // early checks
            if (tokens.count == 0) {
                ReportError("tokens.count == 0");
                return false;
            }

            if (tokens.currIndex == tokens.currentEndIndex) {
                ReportError("tokens.currIndex == tokens.currentEndIndex");
                return false;
            }

            const char* firstTokenStart = nullptr;
            if (tokens.firstTokenStartOffset != nullptr)
                firstTokenStart = tokens.firstTokenStartOffset;
            else
                firstTokenStart = tokens.items[0].start;

            if(IsDoubleOperator(firstTokenStart)/* && exprContext == ExpressionContext::IfCondition*/) { // this only checks the two first characters in the Expression
                ReportError("expr. cannot start with a operator");
                anyError = true;
            }
            if(IsSingleOperator(*firstTokenStart)) {
                ReportError("expr. cannot start with a operator");
                anyError = true;
            }

            int operatorCount, operandCount, leftParenthesisCount, rightParenthesisCount;
            CountOperatorsAndOperands(tokens, operatorCount, operandCount, leftParenthesisCount, rightParenthesisCount, exprContext);

            if (operatorCount >= operandCount) {
                ReportError("double operator(s) detected");
                anyError = true;
            } else if (operatorCount != operandCount - 1) {
                ReportInfo("operatorCount:" + std::to_string(operatorCount) + ", operandCount:" + std::to_string(operandCount) + "\n");
                ReportError("operator(s) missing before/after parenthesis");
                anyError = true;
            }
            if (leftParenthesisCount != rightParenthesisCount) {
                ReportError("mismatch parenthesis detected");
                anyError = true;
            }

            /*ReportInfo("operatorCount:" + std::to_string(operatorCount) +
                    ", operandCount:" + std::to_string(operandCount) +
                    ", leftParenthesisCount:" + std::to_string(leftParenthesisCount));
*/
            if (anyError) return false;


            //int operandIndex = 0;
            bool inOperand = false;
            const char* p = nullptr;
            const char* operandStart = nullptr;
            const char* operandEnd = nullptr;

            int startIndex = tokens.currIndex;
            int endIndex = tokens.currentEndIndex;
            ReportInfo("\nValidateExpression startIndex:" + std::to_string(startIndex) + "\n");
            ReportInfo("ValidateExpression endIndex:" + std::to_string(endIndex) + "\n");
            ReportInfo("ValidateExpression tokens.count:" + std::to_string(tokens.count) + "\n");
            for (int cti = startIndex; cti < endIndex; ++cti) {
                Token& token = tokens.items[cti];
                
                //int a=0;
                //int b = 5/a; // will introduce a crash

                const char* effectiveStart  = nullptr;
                if (cti == startIndex && tokens.firstTokenStartOffset != nullptr) {
                    effectiveStart  = tokens.firstTokenStartOffset;
                    //std::cout << "firstTokenStartOffset was true\n"; 
                } else {
                    effectiveStart  = token.start;
                }

                Token tokToPrint = token;
                tokToPrint.start = effectiveStart;
                ReportTokenInfo(tokToPrint, "checking token:", tokToPrint.ToString().c_str());

                const char* tokenEnd = token.end;
                for (p = effectiveStart ; p < tokenEnd; ++p) {
                    if (IsDoubleOperator(p) /*&& exprContext == ExpressionContext::IfCondition*/) {
                        if (inOperand) {
                            operandEnd = p;
                            Token operand(operandStart, operandEnd);
                            operand.line = token.line;
                            operand.column = token.column + (operandStart-effectiveStart);
                            ValidateOperand(operand, anyError);
                            //operandIndex++;
                            inOperand = false;
                        }
                        ++p; // Skip second char of double op
                    } else if (IsSingleOperator(*p) || *p == '(' || *p == ')' || *p == HAL_JSON_SCRIPTS_EXPRESSIONS_MULTILINE_KEYWORD[0]) {
                        if (inOperand) {
                            operandEnd = p;
                            Token operand(operandStart, operandEnd);
                            operand.line = token.line;
                            operand.column = token.column + (operandStart-effectiveStart);
                            ValidateOperand(operand, anyError);
                            //operandIndex++;
                            inOperand = false;
                        }
                    } else if (!inOperand) {
                        operandStart = p;
                        inOperand = true;
                    }
                }
                if (inOperand) { // this was outside of the last loop
                    operandEnd = p;
                    Token operand(operandStart, operandEnd);
                    operand.line = token.line;
                    operand.column = token.column + (operandStart-effectiveStart);
                    ValidateOperand(operand, anyError);
                    //operandIndex++;
                    inOperand = false;
                }
            }
            if (anyError == false) {
                CalcStackSizes(tokens);
            }
            return !anyError;
        }

        void Expressions::ValidateOperand(const Token& operandToken, bool& anyError, ValidateOperandMode mode) {
            //bool operandIsVariable = OperandIsVariable(operandToken);
#ifdef HAL_JSON_SCRIPTS_EXPRESSIONS_PARSER_SHOW_DEBUG
            std::string msg;
            //if (OperandIsVariable(operandToken)) {
            if (operandToken.ValidNumber() == false) {
                msg = "Variable: Name= ";
                ZeroCopyString funcName = operandToken;
                ZeroCopyString base = funcName.SplitOffHead('#');
                msg += base.ToString();
                if (funcName.Length() != 0) {
                    msg += ", funcName= ";
                    msg += funcName.ToString();
                }
            } else {
                msg = "Const Value: ";
                msg += operandToken.ToString();
            }
            ReportTokenInfo(operandToken, msg.c_str());
#endif
            // return early if not a operand variable
            //if (OperandIsVariable(operandToken) == false)
            if (operandToken.ValidNumber()) return;
                return;

            const char* currChar = ValidOperandVariableName(operandToken);
            if (currChar /*&& *currChar != '\0'*/) {
                std::string msg = "found invalid character <" + std::to_string(*currChar) +
                                "> in operand: " + operandToken.ToString();
                ReportTokenWarning(operandToken, msg.c_str());
                // continue validation but note the error
            }

            ZeroCopyString varOperand = operandToken;
            ZeroCopyString funcName = varOperand;
            varOperand = funcName.SplitOffHead('#');

            if (varOperand.Length() > HAL_UID::Size) {
                std::string msg = varOperand.ToString() + " length > " + std::to_string(HAL_UID::Size);
                
                ReportTokenError(operandToken, "Operand name too long: ", msg.c_str());
                anyError = true;
            }

            UIDPath path(varOperand);
            Device* device = Manager::findDevice(path);
            if (!device) {
                std::string deviceName = varOperand.ToString();
                ReportTokenError(operandToken, "could not find the device: ", deviceName.c_str());
                anyError = true;
                return;
            }
            if (mode == ValidateOperandMode::Read || mode == ValidateOperandMode::ReadWrite) {
                HALOperationResult readResult = HALOperationResult::UnsupportedOperation;
                if (funcName.Length() != 0) {
                    HALValue halValue;
                    HALReadValueByCmd readValueByCmd(halValue, funcName);
                    readResult = device->read(readValueByCmd);
                    if (readResult != HALOperationResult::Success) {
                        anyError = true;
                        if (readResult == HALOperationResult::UnsupportedCommand) {
                            std::string funcNameStr = ": " + funcName.ToString();
                            ReportTokenError(operandToken, HALOperationResultToString(readResult), funcNameStr.c_str());
                        } else {
                            ReportTokenError(operandToken, HALOperationResultToString(readResult), ": read");
                        }
                        
                    }
                } else {
                    HALValue halValue;
                    readResult = device->read(halValue);
                    if (readResult != HALOperationResult::Success) {
                        ReportTokenError(operandToken, HALOperationResultToString(readResult), ": read");
                        anyError = true;
                    }
                }
            }
            if (mode == ValidateOperandMode::Write || mode == ValidateOperandMode::ReadWrite) {
                HALOperationResult writeResult = HALOperationResult::UnsupportedOperation;
                if (funcName.Length() != 0) {
                    HALValue halValue;
                    HALWriteValueByCmd writeValueByCmd(halValue, funcName);
                    writeResult = device->write(writeValueByCmd);
                    if (writeResult != HALOperationResult::Success) {
                        anyError = true;
                        if (writeResult == HALOperationResult::UnsupportedCommand) {
                            std::string funcNameStr = ": " + funcName.ToString();
                            ReportTokenError(operandToken, HALOperationResultToString(writeResult), funcNameStr.c_str());
                        } else {
                            ReportTokenError(operandToken, HALOperationResultToString(writeResult), ": write");
                        }
                        
                    }
                } else {
                    HALValue halValue;
                    writeResult = device->write(halValue);
                    if (writeResult != HALOperationResult::Success) {
                        ReportTokenError(operandToken, HALOperationResultToString(writeResult), ": write");
                        anyError = true;
                    }
                }
            }
        }

        //    ██████  ██████  ███    ██ 
        //    ██   ██ ██   ██ ████   ██ 
        //    ██████  ██████  ██ ██  ██ 
        //    ██   ██ ██      ██  ██ ██ 
        //    ██   ██ ██      ██   ████ 

        std::string Expressions::CalcExpressionToString(int startIndex, int endIndex) {
            std::string out;
            ExpressionToken* calcExprItems = Expressions::rpnOutputStack->items;
            for (int i=startIndex;i<endIndex;i++) {
                out += calcExprItems[i].ToString();
                if (i + 1 < endIndex) out += " ";
            }
            return out;
        }

        std::string Expressions::CalcExpressionToString(const LogicRPNNode* node) {
            std::string out;
            int endIndex = node->calcRPNEndIndex;
            ExpressionToken* calcExprItems = Expressions::rpnOutputStack->items;
            for (int i=node->calcRPNStartIndex;i<endIndex;i++) {
                out += calcExprItems[i].ToString();
                if (i + 1 < endIndex) out += " ";
            }
            return out;
        }

        void Expressions::printLogicRPNNodeTree(const LogicRPNNode* node, int indent) {
            if (!node) return;
            std::string padding(indent * 2, ' '); // 2 spaces per level
            if (node->calcRPNStartIndex != -1) {
                // Leaf node → print calc RPN
                ReportInfo(padding + "- calc: [" + CalcExpressionToString(node) + "]\n");
            } else {
                // Operator node
                ReportInfo(padding + "[" + (node->op?ExpTokenTypeToString(node->op->type):"") + "]\n");
                if (node->childA) printLogicRPNNodeTree(node->childA, indent + 1);
                if (node->childB) printLogicRPNNodeTree(node->childB, indent + 1);
             }
        }

        void Expressions::PrintLogicRPNNodeAdvancedTree(const LogicRPNNode* node, int depth) {
            if (!node) {
                ReportInfo(std::string(depth * 2, ' ') + "Node: nullptr\n");
                return;
            }
            std::string indent(depth * 2, ' ');
            // Print operator
            if (node->op) {
                ReportInfo(indent + "op: " + node->op->ToString() + "\n");  
            } else {
                ReportInfo(indent + "op: nullptr\n");
            }
            // Print calcRPN tokens
            ReportInfo(indent + "calcRPN: ");
            if (node->calcRPNStartIndex != -1) {
                ReportInfo("empty\n");
            } else {
                ReportInfo(CalcExpressionToString(node));
                ReportInfo("\n");
            }
            // Print children
            ReportInfo(indent + "childA:\n");
            if (node->childA)
                PrintLogicRPNNodeAdvancedTree(node->childA, depth + 1);
            else
                ReportInfo(indent + "  nullptr\n");

            ReportInfo(indent + "childB:\n");
            if (node->childB)
                PrintLogicRPNNodeAdvancedTree(node->childB, depth + 1);
            else
                ReportInfo(indent + "  nullptr\n");
        }

        void Expressions::GetGenerateRPNTokensCount_PreCalc(const Tokens& tokens, int& totalCount, int& operatorCount, int&finalOutputCount) {
            int totalCountTemp = 0;
            int operatorCountTemp = 0;
            int finalOutputSizeTemp = 0;

            const int startIndex = tokens.currIndex;
            const int endIndex = tokens.currentEndIndex;
            for (int cti = startIndex; cti < endIndex; cti++) {
                const Token& token = tokens.items[cti];
                const char* tokenStart = nullptr;
                if (cti == startIndex && tokens.firstTokenStartOffset != nullptr) {
                    tokenStart  = tokens.firstTokenStartOffset;
                    //ReportInfo("***************** firstTokenStartOffset was set\n"); 
                } else {
                    tokenStart  = token.start;
                }
                const char* tokenEnd = token.end;

                for (const char* cPtr=tokenStart; cPtr < tokenEnd; cPtr++) {
                    
                    if (*cPtr == '(') {
                        operatorCountTemp++;
                    } else if (*cPtr == ')') {                      
                        
                    } else if ((cPtr + 1) < tokenEnd && IsTwoCharOp(cPtr) != ExpTokenType::NotSet) {
                        if (IsLogicOrCompare(cPtr))
                            finalOutputSizeTemp++;
                        ++totalCountTemp;
                        operatorCountTemp++;
                        cPtr++; // consume the extra char
                    } else if (IsSingleOp(*cPtr) != ExpTokenType::NotSet) {
                        if (IsSingleCompare(*cPtr))
                            finalOutputSizeTemp++;
                        ++totalCountTemp;
                        operatorCountTemp++;
                    } else {
                        // identifier/number
                        while (cPtr < tokenEnd) {
                            if (*cPtr == '(') break;
                            if (*cPtr == ')') break;
                            if (IsSingleOp(*cPtr) != ExpTokenType::NotSet) break;;
                            if ((cPtr + 1) < tokenEnd && IsTwoCharOp(cPtr) != ExpTokenType::NotSet) break;
                            cPtr++;
                        }
                        cPtr--; // adjust for outer loop
                        ++totalCountTemp;
                    }
                }
            }
            totalCount = totalCountTemp;
            operatorCount = operatorCountTemp;
            finalOutputCount = finalOutputSizeTemp;
        }
        int Expressions::GetGenerateRPNTokensCount_DryRun(const Tokens& tokens, int initialSize)
        {
            int maxOperatorUsage = 0;  // track maximum usage for this expression

            // Dry-run op stack (store only TokenType, not full ExpressionToken*)
            ExpTokenType* opStack = new ExpTokenType[initialSize];
            int opStackIndex = 0;      // current operator stack depth
            const int startIndex = tokens.currIndex;
            const int endIndex = tokens.currentEndIndex;
            for (int cti = startIndex; cti < endIndex; cti++) {
                const Token& token = tokens.items[cti];
                const char* tokenStart = nullptr;
                if (cti == startIndex && tokens.firstTokenStartOffset != nullptr) {
                    tokenStart  = tokens.firstTokenStartOffset;
                    //ReportInfo("***************** firstTokenStartOffset was set\n"); 
                } else {
                    tokenStart  = token.start;
                }
                const char* tokenEnd = token.end;

                for (const char* cPtr=tokenStart; cPtr < tokenEnd; cPtr++) {
                    //char c = token[j];

                    // Parentheses
                    if (*cPtr == '(') {
                        opStack[opStackIndex++] = ExpTokenType::LeftParenthesis;

                        if (opStackIndex > maxOperatorUsage) maxOperatorUsage = opStackIndex;
                    }
                    else if (*cPtr == ')') {
                        // Pop until LeftParenthesis is found
                        while (opStackIndex != 0) {
                            ExpTokenType top = opStack[opStackIndex - 1];
                            if (top == ExpTokenType::LeftParenthesis) break;
                            opStackIndex--;
                        }
                        if (opStackIndex != 0)
                            opStackIndex--; // discard LeftParenthesis
                    }
                    else {
                        // Detect operator type
                        ExpTokenType twoCharOpType = ExpTokenType::NotSet;
                        if ((cPtr + 1) < tokenEnd) twoCharOpType = IsTwoCharOp(cPtr);

                        if (twoCharOpType != ExpTokenType::NotSet) {
                            // Pop higher or equal precedence operators
                            while (opStackIndex != 0) {
                                ExpTokenType top = opStack[opStackIndex - 1];
                                if (top == ExpTokenType::LeftParenthesis) break;
                                if (getPrecedence(top) < getPrecedence(twoCharOpType)) break;
                                opStackIndex--;
                            }
                            opStack[opStackIndex++] = twoCharOpType;
                            if (opStackIndex > maxOperatorUsage) maxOperatorUsage = opStackIndex;
                            cPtr++; // consume extra char
                            continue;
                        }

                        // Single-character operator
                        ExpTokenType oneCharOpType = IsSingleOp(*cPtr);
                        if (oneCharOpType != ExpTokenType::NotSet) {
                            while (opStackIndex != 0) {
                                ExpTokenType top = opStack[opStackIndex - 1];
                                if (top == ExpTokenType::LeftParenthesis) break;
                                if (getPrecedence(top) < getPrecedence(oneCharOpType)) break;
                                opStackIndex--;
                            }
                            opStack[opStackIndex++] = oneCharOpType;
                            if (opStackIndex > maxOperatorUsage) maxOperatorUsage = opStackIndex;
                        }
                        else {
                            // Operand / identifier
                            while (cPtr < tokenEnd) {
                                if (*cPtr == '(') break;
                                if (*cPtr == ')') break;
                                if ((cPtr + 1) < tokenEnd && IsTwoCharOp(cPtr) != ExpTokenType::NotSet) break;
                                if (IsSingleOp(*cPtr) != ExpTokenType::NotSet) break;
                                cPtr++;
                            }
                            cPtr--; // adjust for outer loop increment
                        }
                    }
                }
            }
            delete[] opStack;
            return maxOperatorUsage;
        }

        ExpressionTokens* Expressions::GenerateRPNTokens(Tokens& tokens) {
            //std::string msg = "\n***** GenerateRPNTokens: "; msg+= tokens.SliceToString(); msg+= "\n";
            //ReportInfo(msg);
            // development test only
            int maxOperatorUsage = 0;
            // alias
            ExpressionToken* outTokenItems = rpnOutputStack->items;
            // current stack indicies
            int opStackIndex = 0;
            int outTokensIndex = 0;
            
            const int startindex = tokens.currIndex;
            const int endIndex = tokens.currentEndIndex;
            if (endIndex > tokens.count) {
                printf("\n !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! MASTER ERROR: endIndex >= tokens.count @ GenerateRPNTokens\n");
                return nullptr;
            }
            //printf("\nGenerateRPNTokens - startindex:%d, endIndex:%d\n\n",startindex,endIndex);
            
            // consume current tokens here so we don't forget and to clearly mark what is happend
            tokens.currIndex = endIndex; 

            for (int cti = startindex; cti < endIndex; cti++) {
                const Token& token = tokens.items[cti];
                if (token.type == TokenType::Ignore) continue;
                
                const char* tokenStart = nullptr;
                if (cti == startindex && tokens.firstTokenStartOffset != nullptr) {
                    tokenStart  = tokens.firstTokenStartOffset;
                    //ReportInfo("***************** firstTokenStartOffset was set\n"); 
                } else {
                    tokenStart  = token.start;
                }
                const char* tokenEnd = token.end;
                //const int tokenStrLength = token.Length();
                for (const char* cPtr=tokenStart; cPtr < tokenEnd; cPtr++) {
                    
                    // Parentheses
                    if (*cPtr == '(') {
                        // Push current operator as 'new' item
                        opStack[opStackIndex++].Set(cPtr, 1, ExpTokenType::LeftParenthesis);
                        if (opStackIndex > maxOperatorUsage) maxOperatorUsage = opStackIndex; // debug only
                    }
                    else if (*cPtr == ')') {
                        // Pop until LeftParenthesis is found
                        while (opStackIndex != 0)
                        {
                            ExpressionToken& top = opStack[opStackIndex - 1];
                            if (top.type == ExpTokenType::LeftParenthesis) break;
                            outTokenItems[outTokensIndex++] = top;
                            
                            opStackIndex--;
                        }

                        if (opStackIndex != 0)
                            opStackIndex--; // discard the LeftParenthesis
                        else
                            ReportError("Mismatched parenthesis"); // should never happend
                    }
                    else {
                        ExpTokenType twoCharOpType = ExpTokenType::NotSet;
                        if ((cPtr + 1) < tokenEnd) twoCharOpType = IsTwoCharOp(cPtr);
                        
                        if (twoCharOpType != ExpTokenType::NotSet) {
                            
                            // While there's an operator on top of the opStack with greater or equal precedence
                            while (opStackIndex != 0)
                            {
                                ExpressionToken& top = opStack[opStackIndex - 1];
                                if (top.type == ExpTokenType::LeftParenthesis) break;
                                if (getPrecedence(top.type) < getPrecedence(twoCharOpType)) break;
                                outTokenItems[outTokensIndex++] = top;
                                opStackIndex--;
                            }
                            // Push current operator as 'new' item
                            opStack[opStackIndex++].Set(cPtr, 2, twoCharOpType);
                            if (opStackIndex > maxOperatorUsage) maxOperatorUsage = opStackIndex; // debug only
                            cPtr++; // consume one extra token
                            continue;
                        }

                        // Single-character operator
                        ExpTokenType oneCharOpType = IsSingleOp(*cPtr);
                        if (oneCharOpType != ExpTokenType::NotSet) {
                            
                            // While there's an operator on top of the opStack with greater or equal precedence
                            while (opStackIndex != 0)
                            {
                                ExpressionToken& top = opStack[opStackIndex - 1];
                                if (top.type == ExpTokenType::LeftParenthesis) break;
                                if (getPrecedence(top.type) < getPrecedence(oneCharOpType)) break;
                                outTokenItems[outTokensIndex++] = top;
                                opStackIndex--;
                            }
                            // Push current operator as 'new' item
                            opStack[opStackIndex++].Set(cPtr, 1, oneCharOpType);
                            if (opStackIndex > maxOperatorUsage) maxOperatorUsage = opStackIndex; // debug only
                        }
                        else {
                            // Identifier / number
                            //int startIdx = j;
                            const char* start = cPtr;
                            for (;cPtr < tokenEnd;cPtr++) {
                                if (*cPtr == '(' || *cPtr == ')') break;
                                if (((cPtr + 1) < tokenEnd && IsTwoCharOp(cPtr)!=ExpTokenType::NotSet)) break;
                                if (IsSingleOp(*cPtr) != ExpTokenType::NotSet) break;
                            }
                            ZeroCopyString zcStr(start, cPtr);
                            bool validNumber = zcStr.ValidNumber();
                            ExpTokenType type = validNumber?ExpTokenType::ConstValOperand:ExpTokenType::VarOperand;
                            outTokenItems[outTokensIndex++].Set(start, cPtr, type);
                            cPtr--; // as the current loop will incr after this 

                        }
                    }
/*
                    ReportInfo("outStack:");
                    for (int i=0;i<outTokensIndex;i++) {
                        ReportInfo(outTokenItems[i]->ToString() + " ");
                    }
                    ReportInfo("\n");
                    ReportInfo("opStack:");
                    for (int i=0;i<opStackIndex;i++) {
                        ReportInfo(opStack[i]->ToString() + " ");
                    }
                    ReportInfo("\n");
*/
                }
            }
            // After loop: pop remaining ops
            while (opStackIndex != 0)
                outTokenItems[outTokensIndex++] = opStack[--opStackIndex];

            ReportInfo("GenerateRPNTokens used: " + std::to_string(outTokensIndex) + " of " + std::to_string(rpnOutputStackNeededSize) + "\n");
            ReportInfo("GenerateRPNTokens used op: " + std::to_string(maxOperatorUsage) + " of " + std::to_string(opStackSizeNeededSize) + "\n");

            rpnOutputStack->currentCount = outTokensIndex;

            // define if this expression contains logic operators
            // this allows the tree builder to have a single ptr to the rpnArray as the context
            rpnOutputStack->containLogicOperators = false;
            // going backwards as that is the fastest way of detecting logic operators
            // as they mostly appear at the end
            for (int i=outTokensIndex-1;i>=0;i--) { 
                ExpressionToken& tok = rpnOutputStack->items[i];
                if (tok.type == ExpTokenType::LogicalAnd || tok.type == ExpTokenType::LogicalOr) {
                    rpnOutputStack->containLogicOperators = true;
                    break; //  we only need to check if there is one
                }
            }
            return rpnOutputStack;
        }

        
        
        
        
        /** 
         * Development test functions
         * TODO. make a copy of this that produce the exec format
         */
        LogicRPNNode* Expressions::BuildLogicTree(ExpressionTokens* tokens)
        {
            int tokensCount = tokens->currentCount;

            int stackPoolIndex = 0;
            int stackIndex = 0;
            int stackMaxUsed = 0;
  
            int currentCalcStartIndex = -1;
            
            for (int i=0;i<tokensCount;i++) {

                ExpressionToken& tok = tokens->items[i];
                //if (tok.type == ExpTokenType::NotSet) break;
                if (tok.type == ExpTokenType::LogicalAnd || tok.type == ExpTokenType::LogicalOr) {
                    // first flush pending calc as a leaf
                    if (currentCalcStartIndex != -1) {

                        LogicRPNNode& newNode = logicRPNNodeStackPool[stackPoolIndex++]; //  get next item from the pool
                        newNode.Set(tokens, currentCalcStartIndex, i); // here it should only be i as the logic operator should not be included
                        logicRPNNodeStack[stackIndex++] = &newNode; // push item
                        currentCalcStartIndex = -1; // clear
                    
                        if (stackIndex > stackMaxUsed) stackMaxUsed = stackIndex;
                    }
                    ReportInfo("BuildLogicTree stack size:" + std::to_string(stackIndex) + "\n");
                    // This should never happen under normal use.
                    // It can occur if logical and arithmetic expressions are improperly combined,
                    // e.g., (a == 0 || b == 1) + 2, which is invalid.
                    if (stackIndex < 2)
                        throw std::runtime_error("LogicRPN: not enough operands for logic op");

                    LogicRPNNode* rhs = logicRPNNodeStack[--stackIndex];
                    LogicRPNNode* lhs = logicRPNNodeStack[--stackIndex];
                    LogicRPNNode& newNode = logicRPNNodeStackPool[stackPoolIndex++]; //  get next item from the pool
                    newNode.Set(&tok, lhs, rhs);
                    logicRPNNodeStack[stackIndex++] = &newNode; // push item
                    if (stackIndex > stackMaxUsed) stackMaxUsed = stackIndex;
                }
                else {
                    if (currentCalcStartIndex == -1) currentCalcStartIndex = i;
                    

                    // detect end of a comparison (= leaf boundary)
                    if (tok.AnyType(compareOperators)) {
                        LogicRPNNode& newNode = logicRPNNodeStackPool[stackPoolIndex++]; //  get next item from the pool
                        newNode.Set(tokens, currentCalcStartIndex, i+1); // i+1 as the compare operator needs to be included
                        logicRPNNodeStack[stackIndex++] = &newNode; // push item
                        currentCalcStartIndex = -1; // clear
                        if (stackIndex > stackMaxUsed) stackMaxUsed = stackIndex;
                    }
                }
            }

            if (currentCalcStartIndex != -1) {
                LogicRPNNode& newNode = logicRPNNodeStackPool[stackPoolIndex++];
                newNode.Set(tokens, currentCalcStartIndex, tokensCount);
                logicRPNNodeStack[stackIndex++] = &newNode; // push item
                if (stackIndex > stackMaxUsed) stackMaxUsed = stackIndex;
                currentCalcStartIndex = -1; // clear
            }

            ReportInfo("BuildLogicTree - used " + std::to_string(stackMaxUsed) + " of " + std::to_string(finalOutputStackNeededSize) + "\n");
            if (stackIndex != 1) {
                std::string msg = PrintExpressionTokensOneRow(*tokens, 0, tokens->currentCount);
                throw std::runtime_error("LogicRPN - unbalanced tree: " + std::to_string(stackIndex) + msg);
            }
                

            // note. logicRPNNodeStackPool is not owned and cannot be deleted here
            // note. logicRPNNodeStack is not owned and cannot be deleted here
            return logicRPNNodeStack[stackIndex-1];
        }
    }
}