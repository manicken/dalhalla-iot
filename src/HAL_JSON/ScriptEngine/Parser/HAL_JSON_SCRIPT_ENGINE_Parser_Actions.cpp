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

#include "HAL_JSON_SCRIPT_ENGINE_Parser_Actions.h"
#include "../HAL_JSON_SCRIPT_ENGINE_Reports.h"
#include "../HAL_JSON_SCRIPT_ENGINE_Expression_Parser.h"

namespace HAL_JSON {
    namespace ScriptEngine {
        namespace Parser {
            namespace Actions {
                AssignmentParts g_assignmentParts;  // single reusable instance

                const ScriptTokenType actionStartTypes[] = {ScriptTokenType::ActionSeparator, ScriptTokenType::And, ScriptTokenType::Else, ScriptTokenType::EndIf, ScriptTokenType::Then, ScriptTokenType::NotSet};
                const ScriptTokenType actionEndTypes[] =   {ScriptTokenType::ActionSeparator, ScriptTokenType::And, ScriptTokenType::Else, ScriptTokenType::EndIf, ScriptTokenType::If, ScriptTokenType::ElseIf, ScriptTokenType::EndOn, ScriptTokenType::NotSet};

                bool MergeActions(ScriptTokens& _tokens) {
                    ScriptToken* tokens = _tokens.items;
                    int tokenCount = _tokens.count;
                    int level = 0;
                    for (int i = 0; i < tokenCount; ++i) {
                        ScriptToken& token = tokens[i];

                        if (!token.AnyType(actionStartTypes)) {
                            if (token.type == ScriptTokenType::If || token.type == ScriptTokenType::On)
                                level++;
                            continue;
                        }
                        if (token.type == ScriptTokenType::And) token.type = ScriptTokenType::Ignore;
                        else if (token.type == ScriptTokenType::ActionSeparator) token.type = ScriptTokenType::Ignore;
                        if (token.type == ScriptTokenType::EndIf || token.type == ScriptTokenType::EndOn) { 
                            level--;
                            if (level == 0) {
                                token.ReportTokenInfo("level is zero");
                                continue; // skip start endif if at root level
                            }
                        }

                        int start = i + 1;
                        int end = -1;

                        for (int j = start; j < tokenCount; ++j) {
                            if (tokens[j].type == ScriptTokenType::On) break;
                            if (tokens[j].AnyType(actionEndTypes))
                            {    
                                end = j;
                                break;
                            }
                        }

                        if (end == -1 || start == end) {
                            continue; // malformed or empty block, skip safely
                        }

                        int j = start;
                        while (j < end) {
                            int currentLine = tokens[j].line;
                            int lineTokenCount = 0;

                            // Count tokens for current logical action line, respecting HAL_JSON_SCRIPTS_EXPRESSIONS_MULTILINE_KEYWORD continuation
                            while (j + lineTokenCount < end) {
                                // If token line is current line, include it
                                if (tokens[j + lineTokenCount].line == currentLine) {
                                    lineTokenCount++;
                                }
                                // If last token on current line is HAL_JSON_SCRIPTS_EXPRESSIONS_MULTILINE_KEYWORD, include next line tokens
                                else if (lineTokenCount > 0 && tokens[j + lineTokenCount - 1].type == ScriptTokenType::ActionJoiner) { // .Equals(HAL_JSON_SCRIPTS_EXPRESSIONS_MULTILINE_KEYWORD)) {
                                    // extend currentLine to next line to continue merging
                                    currentLine = tokens[j + lineTokenCount].line;
                                    lineTokenCount++;
                                } else {
                                    break;
                                }
                            }
                            tokens[j].MarkTokenGroup(lineTokenCount, ScriptTokenType::Action);
                            j += lineTokenCount;
                        }
                        i = end - 1;
                    }
                    return true;
                }

                bool VerifyActionBlocks(ScriptTokens& _tokens) {
                    ScriptToken* tokens = _tokens.items;
                    int tokenCount = _tokens.count;
                    bool anyError = false;
                    for (int i = 0; i < tokenCount; ++i) {
                        ScriptToken& token = tokens[i];
                        // 'TokenType::Action' is set only on the root token of an action (whether single-token or merged multi-token).
                        // Subtokens inside merged tokens do not have the type TokenType::Action, instead they have the type TokenType::Merged
                        if (token.type != ScriptTokenType::Action) continue;

                        ScriptTokens expressionTokens;
                        expressionTokens.items = &token;
                        if (token.itemsInBlock != 0) {
                            expressionTokens.count = token.itemsInBlock;
                        } else {
                            expressionTokens.count = 1;
                        }
                        // first check if there is any currently unsupported AssignKeywords
                        const char* match = nullptr;
                        //const char* matchKeyword = nullptr;
                        //int firstAssignmentOperatorTokenIndex = -1;
                        ScriptToken* firstAssignmentOperatorToken = nullptr;
                        const char* firstAssignmentOperator = nullptr;
                        const char* firstCompoundAssignmentOperator = nullptr;
                        bool foundAdditionalAssignmentOperators = false;
                        
                        for (int j = 0; j < expressionTokens.count; ++j) {
                            ScriptToken& exprToken = expressionTokens.items[j];
                            const char* searchStart = exprToken.start;
                            //std::cout << "searching:" << token.ToString() << "\n";
                            do {
                                match = exprToken.FindChar('=', searchStart);
                                if (match) {
                                    if (firstAssignmentOperator) {
                                        foundAdditionalAssignmentOperators = true;
                                        anyError = true;
                                        Token reportToken;
                                        reportToken.line = exprToken.line;
                                        reportToken.column = exprToken.column + (match - exprToken.start);
                                        reportToken.ReportTokenError("Found additional assignment keyword");
                                        
                                    } else {
                                        firstAssignmentOperator = match;
                                        firstAssignmentOperatorToken = &exprToken;
                                        //firstAssignmentOperatorTokenIndex = j;
                                        const char* prevChar = match-1;
                                        if (exprToken.ContainsPtr(prevChar) && Expressions::IsSingleOperator(*prevChar)) {
                                            // this mean that we found a Compound Assignment Operator
                                            firstCompoundAssignmentOperator = prevChar;
                                        }
                                    }
                                
                                    
                                    // Advance search start
                                    searchStart = match + 1;

                                    if (searchStart >= exprToken.end) break;
                                }
                            
                            } while (match);
                        }
                        // have:
                        // const char* firstAssigmentOperator // is set when a assigment operator is found
                        // const char* firstCompundAssignmentOperator // is set when a compund assigment operator is found
                        // bool foundAdditionalAssigmentOperators
                        
                        if (foundAdditionalAssignmentOperators) {
                            // error reporting  is taken care of above
                            continue; // skip for now as it would be hard to extract anything from such string
                        }
                        if (firstAssignmentOperator == nullptr) {
                            // invalid action line if no assigmend operator is found
                            // maybe in future i can support direct function calls like: somefunc(var2)
                            token.ReportTokenError("Did not find any assignment keyword");
                            anyError = true;
                            continue; 
                        }
                        const char* firstAssigmentOperatorStart = nullptr;

                        if (firstCompoundAssignmentOperator) {
                            char ch = *firstCompoundAssignmentOperator;
                            
                            if (ch == '<' || ch == '>') {
                                const char* prevChar = firstCompoundAssignmentOperator - 1;
                                // Handle shift compound assignment (<<= or >>=)
                                bool prevIsValid = firstAssignmentOperatorToken->ContainsPtr(prevChar) &&
                                                (*prevChar == '<' || *prevChar == '>');
                                
                                if (prevIsValid) {
                                    firstAssigmentOperatorStart = prevChar; // valid <<= or >>=
                                } else {
                                    anyError = true;
                                    token.ReportTokenError("missing additional < or > in compound shift assignment keyword");
                                    firstAssigmentOperatorStart = firstCompoundAssignmentOperator;
                                }
                            } else {
                                // Not a shift op, treat as normal compound assignment like +=, -=
                                firstAssigmentOperatorStart = firstCompoundAssignmentOperator;
                            }
                        } else {
                            firstAssigmentOperatorStart = firstAssignmentOperator;
                        }
                        ScriptToken zcLHS_AssignmentOperand;
                        ScriptTokens zcRHS_AssignmentOperands;
                        zcLHS_AssignmentOperand.start = token.start;
                        zcLHS_AssignmentOperand.line = token.line;
                        zcLHS_AssignmentOperand.column = token.column;

                        if (token == *firstAssignmentOperatorToken) { 
        #if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
                            std::cout << "(token.start == firstAssignmentOperatorToken->start):" << token.ToString() << "\n";
        #endif
                            // this mean that the assigmentOperator is in the first token
                            // someVar= 5 or someVar=5(if this then token.itemsInBlock == 0)
                            if (token.itemsInBlock < 2) {
                                zcRHS_AssignmentOperands.items = expressionTokens.items;
                                zcRHS_AssignmentOperands.count = 1;
                                zcRHS_AssignmentOperands.firstTokenStartOffset = firstAssignmentOperator + 1;
                            } else {
                                zcRHS_AssignmentOperands.items = expressionTokens.items+1;
                                zcRHS_AssignmentOperands.count = expressionTokens.count-1;
                            }
                            zcLHS_AssignmentOperand.end = firstAssigmentOperatorStart;
                        } else {
                            // this mean that the assigmentOperator is
                            // separated from the first operand

                            // someVar =5 or someVar +=5
                            if (firstAssignmentOperatorToken->ContainsPtr(firstAssignmentOperator+1)) {
                                //std::cout << firstAssignmentOperatorToken->ToString() << " -> ContainsPtr("<<firstAssignmentOperator+1<<"): " << "\n";
                                zcRHS_AssignmentOperands.items = firstAssignmentOperatorToken;
                                zcRHS_AssignmentOperands.count = expressionTokens.count-1;
                                zcRHS_AssignmentOperands.firstTokenStartOffset = firstAssignmentOperator + 1;
                                ZeroCopyString zcTemp(firstAssignmentOperator + 1, firstAssignmentOperator + 2);
                            }
                            // someVar = 6 or someVar += 5 
                            else {
                                zcRHS_AssignmentOperands.items = firstAssignmentOperatorToken+1;
                                zcRHS_AssignmentOperands.count = expressionTokens.count-2;
                            }
                            zcLHS_AssignmentOperand.end = token.end;
                        }
        #if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
                        std::cout << "zcLHS_AssignmentOperand: " << zcLHS_AssignmentOperand.ToString() << "\n";
        #endif
                        if (firstCompoundAssignmentOperator) {
                            Expressions::ValidateOperand(zcLHS_AssignmentOperand, anyError, ValidateOperandMode::ReadWrite);
                        }
                        else {
                            Expressions::ValidateOperand(zcLHS_AssignmentOperand, anyError, ValidateOperandMode::Write);
                        }
                        // use the following to validate the right side of the expression
                        zcRHS_AssignmentOperands.currentEndIndex = zcRHS_AssignmentOperands.count;
                        if (Expressions::ValidateExpression(zcRHS_AssignmentOperands, ExpressionContext::Assignment) == false) {
                            token.ReportTokenError("Expressions::ValidateExpression fail");
                            anyError = true;
                        } 

                    }
                    return anyError == false;
                }
                AssignmentParts* ExtractAssignmentParts(ScriptTokens& tokens) {
                    g_assignmentParts.Clear();

                    ScriptToken& currentStartToken = tokens.Current();
                    int currentStartTokenIndex = tokens.currIndex;
                    ScriptToken* tokensItems = tokens.items;
                    int startIndex = tokens.currIndex;
                    int endIndex   = startIndex + currentStartToken.itemsInBlock;
                    tokens.currIndex = endIndex; // consume tokens beforehand so we don't forget

                    // Track assignment operator info
                    const char* foundAssignmentOperator = nullptr;
                    ScriptToken* foundAssignmentOperatorToken = nullptr;
                    const char* foundCompoundAssignmentOperator = nullptr;

                    // Scan tokens in the current block for the assigment operator
                    // to get it's position
                    for (int i = startIndex; i < endIndex; ++i) {
                        ScriptToken& exprToken = tokensItems[i];
                        if (exprToken.type == ScriptTokenType::Ignore) continue;
                        const char* match = exprToken.FindChar('=');
                        if (match == nullptr) continue; 

                        foundAssignmentOperator = match;
                        foundAssignmentOperatorToken = &exprToken;

                        // check for compound assignment
                        const char* prevChar = match - 1;
                        if (exprToken.ContainsPtr(prevChar) &&
                            Expressions::IsSingleOperator(*prevChar)) {
                            // as this is validated beforehand we can safely assume that 
                            // a additional < or > exists
                            if (*prevChar == '<' || *prevChar == '>') 
                                prevChar--; // if it's leftwhift or rightshift decrease the pointer
                            foundCompoundAssignmentOperator = prevChar;
                        }
                        break; // break here as we found the =
                    }
                    // have:
                    // const char* firstAssignmentOperator // is set when a assigment operator is found
                    // const char* firstCompoundAssignmentOperator // is set when a compound assigment operator is found

                    if (!foundAssignmentOperator) {
                        // no operator found: just return empty
                        currentStartToken.ReportTokenError("!!!!!!!!!!!!!!!!!!!!!!!! firstAssignmentOperator not found");
                        return &g_assignmentParts;
                    }

                    // Decide operator start
                    const char* foundAssigmentOperatorStart = foundCompoundAssignmentOperator
                                        ? foundCompoundAssignmentOperator
                                        : foundAssignmentOperator;

                    g_assignmentParts.lhs.start = currentStartToken.start;
                    g_assignmentParts.lhs.line = currentStartToken.line;
                    g_assignmentParts.lhs.column = currentStartToken.column;
                    g_assignmentParts.op = *foundAssigmentOperatorStart;

                    g_assignmentParts.rhs.items = tokens.items;
                    g_assignmentParts.rhs.count = tokens.count;
                    g_assignmentParts.rhs.currentEndIndex = currentStartTokenIndex + currentStartToken.itemsInBlock;

                    if (currentStartToken == *foundAssignmentOperatorToken) {
                        // this mean that the assigmentOperator is in the first token
                        // finalize the lhs first
                        g_assignmentParts.lhs.end = foundAssigmentOperatorStart;
        #if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
                        std::cout << "(currentStartToken.start == foundAssignmentOperatorToken->start):" << currentStartToken.ToString() << "\n";
        #endif
                        // someVar=5 cases
                        if (currentStartToken.itemsInBlock < 2) { 
                            g_assignmentParts.rhs.currIndex = currentStartTokenIndex;
                            g_assignmentParts.rhs.firstTokenStartOffset = foundAssignmentOperator + 1;
                        }
                        // someVar= 5 cases
                        else { 
                            g_assignmentParts.rhs.currIndex = currentStartTokenIndex + 1;
                        }
                    }
                    else // currentStartToken != *foundAssignmentOperatorToken
                    {
                        // this mean that the assigmentOperator is separated from the first operand
                        // finalize the lhs first
                        g_assignmentParts.lhs.end = currentStartToken.end;

                        // someVar =5 or someVar +=5
                        if (foundAssignmentOperatorToken->ContainsPtr(foundAssignmentOperator+1)) {
                            // this mean that there are characters after the assignment operator
                            g_assignmentParts.rhs.firstTokenStartOffset = foundAssignmentOperator + 1;
                            g_assignmentParts.rhs.currIndex = currentStartTokenIndex+1;
                        }
                        // someVar = 6 or someVar += 5 
                        else {
                            g_assignmentParts.rhs.currIndex = currentStartTokenIndex+2;
                        }
                    }

                    return &g_assignmentParts;
                }
            }
        }
    }
}