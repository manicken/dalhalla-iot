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
#include "HAL_JSON_SCRIPT_ENGINE_Parser_Expressions.h"

namespace HAL_JSON {
    namespace ScriptEngine {
        namespace Parser {
            namespace Actions {
                //AssignmentParts g_assignmentParts;  // single reusable instance
                ScriptToken AssignmentParts::lhs = {};
                char AssignmentParts::op = 0;
                ScriptTokens AssignmentParts::rhs = {};

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
                        if (token.type != ScriptTokenType::Action) continue;
                        // set current Token
                        _tokens.currIndex = i;
                        if (false == ExtractAssignmentParts(_tokens)) {
                            token.ReportTokenError("Invalid assignment in action block");
                            anyError = true;
                            continue;
                        }
                        printf("AssignmentParts::rhs= %s (%d)\n",AssignmentParts::rhs.SliceToString().c_str(), AssignmentParts::rhs.SliceTokenCount());
                        // Validate LHS
                        // here set opMode to ReadWrite on compound operators otherwise write only
                        ValidateOperandMode opMode = ValidateOperandMode::UnSet;

                        if (AssignmentParts::op == '(') {
                            opMode = ValidateOperandMode::Exec;
                        } else if (AssignmentParts::op == '=') {
                            opMode = ValidateOperandMode::Write;
                        } else { // compound operators
                            opMode = ValidateOperandMode::ReadWrite;
                        }
                        
                        // validate LHS even if rhs could fail
                        Expressions::ValidateOperand(AssignmentParts::lhs, anyError, opMode);
                        
                        if (opMode == ValidateOperandMode::Exec) {
                            // Exec currently do not support parameters
                            // and probably will never do either as there is currently no use
                            // also if there is parameters given then we can safely just ignore them
                            continue;
                        }
                        if (Expressions::IsExpressionEmpty(AssignmentParts::rhs)) {
                            token.ReportTokenError("RHS expression cannot be empty");
                            anyError = true;
                            continue;
                        }
                        // Validate RHS
                        if (false == Expressions::ValidateExpression(AssignmentParts::rhs)) {
                            token.ReportTokenError("RHS expression validation failed");
                            anyError = true;
                        }

                    } // for loop

                    return !anyError;
                }

                bool ExtractAssignmentParts(ScriptTokens& tokens) {
                    AssignmentParts::Clear();

                    ScriptToken& currentStartToken = tokens.Current();
                    ScriptToken* tokensItems = tokens.items;
                    int startIndex = tokens.currIndex;
                    int endIndex   = startIndex + currentStartToken.itemsInBlock;
                    tokens.currIndex = endIndex; // consume tokens beforehand so we don't forget

                    // Track assignment operator info
                    const char* foundAssignmentOperator = nullptr;
                    ScriptToken* foundAssignmentOperatorToken = nullptr;
                    const char* foundCompoundAssignmentOperator = nullptr;
                    int assignmentTokenIndex = startIndex;
                    // Scan tokens in the current block for the assigment operator
                    // to get it's position
                    for (int i = startIndex; i < endIndex; ++i) {
                        ScriptToken& exprToken = tokensItems[i];
                        if (exprToken.type == ScriptTokenType::Ignore) continue;
                        const char* match = exprToken.FindChar('=');
                        if (match == nullptr) continue; 

                        foundAssignmentOperator = match;
                        foundAssignmentOperatorToken = &exprToken;
                        assignmentTokenIndex = i;
                        break;
                    }
                    if (!foundAssignmentOperator) {
                        // TODO make this check for left parenthesis
                        // but only in the first token where it should be as: duid () is invalid
                        const char* match = tokensItems[startIndex].FindChar('(');
                        if (match == nullptr) {
                            // no operator found, no function call found
                            currentStartToken.ReportTokenError("!!!!!!!!!!!!!!!!!! (assignment operator)/(func call)  not found");
                            return false;
                        }
                       
                        // found function call statement
                        foundAssignmentOperator = match;
                        foundAssignmentOperatorToken = &tokensItems[startIndex];
                        
                    }
                    else {
                        // check for compound assignment
                        const char* prevChar = foundAssignmentOperator - 1;
                        if (foundAssignmentOperatorToken->ContainsPtr(prevChar) && Expressions::IsSingleOperator(*prevChar)) {

                            // Handle <<=
                            if (*prevChar == '<' && foundAssignmentOperatorToken->ContainsPtr(prevChar - 1) && *(prevChar - 1) == '<') {
                                foundCompoundAssignmentOperator = prevChar - 1; // start of '<<='
                            }
                            // Handle >>=
                            else if (*prevChar == '>' && foundAssignmentOperatorToken->ContainsPtr(prevChar - 1) && *(prevChar - 1) == '>') {
                                foundCompoundAssignmentOperator = prevChar - 1; // start of '>>='
                            }
                            // Handle all other compound ops like +=, -=, *=, etc.
                            else {
                                foundCompoundAssignmentOperator = prevChar; // start of the two-character op
                            }

                            // Optional: validate that we’re not missing something like "a<="
                            if (*prevChar == '<' || *prevChar == '>') {
                                // Ensure previous char is also same operator, otherwise invalid
                                if (!foundAssignmentOperatorToken->ContainsPtr(prevChar - 1) || *(prevChar - 1) != *prevChar) {
                                    foundAssignmentOperatorToken->ReportTokenError("Invalid compound shift assignment (expected <<= or >>=)");
                                    return false;
                                }
                            }
                        }
                    }                  
                    // here we have have:
                    // const char* firstAssignmentOperator // is set when a assigment operator is found
                    // const char* foundCompoundAssignmentOperator // is set when a compound assigment operator is found

                    // Decide operator start
                    const char* foundAssigmentOperatorStart = foundCompoundAssignmentOperator
                                        ? foundCompoundAssignmentOperator
                                        : foundAssignmentOperator;
                    AssignmentParts::lhs = currentStartToken;
                    //AssignmentParts::lhs.start = currentStartToken.start;
                    //AssignmentParts::lhs.line = currentStartToken.line;
                    //AssignmentParts::lhs.column = currentStartToken.column;
                    AssignmentParts::op = *foundAssigmentOperatorStart;

                    AssignmentParts::rhs.items = tokens.items;
                    AssignmentParts::rhs.count = tokens.count;
                    AssignmentParts::rhs.currentEndIndex = startIndex + currentStartToken.itemsInBlock;
                    
                    if (*foundAssigmentOperatorStart == '(') {
                        AssignmentParts::lhs.end = foundAssigmentOperatorStart;
                        AssignmentParts::rhs.currIndex = startIndex;
                        AssignmentParts::rhs.firstTokenStartOffset = foundAssigmentOperatorStart;
                    }
                    else if (currentStartToken == *foundAssignmentOperatorToken) {
                        // this mean that the assigmentOperator is in the first token
                        // finalize the lhs first
                        AssignmentParts::lhs.end = foundAssigmentOperatorStart;
        #if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
                        std::cout << "(currentStartToken.start == foundAssignmentOperatorToken->start):" << currentStartToken.ToString() << "\n";
        #endif
                        // someVar=5 cases
                        if (currentStartToken.itemsInBlock < 2) { 
                            AssignmentParts::rhs.currIndex = startIndex;
                            AssignmentParts::rhs.firstTokenStartOffset = foundAssignmentOperator + 1;
                        }
                        // someVar= 5 cases
                        else { 
                            AssignmentParts::rhs.currIndex = startIndex + 1;
                        }
                    }
                    else // currentStartToken != *foundAssignmentOperatorToken
                    {
                        // this mean that the assigmentOperator is separated from the first operand
                        // finalize the lhs first, 
                        //AssignmentParts::lhs.end = currentStartToken.end;

                        // someVar =5 or someVar +=5
                        if (foundAssignmentOperatorToken->ContainsPtr(foundAssignmentOperator+1)) {
                            if (assignmentTokenIndex != startIndex) {
                                currentStartToken.ReportTokenError("!!!!!!!!!!!!!!!!!! ExtractAssignmentParts expected assignmentTokenIndex mismatch");
                                return false;
                            }
                            // this mean that there are characters after the assignment operator
                            AssignmentParts::rhs.firstTokenStartOffset = foundAssignmentOperator + 1;
                            AssignmentParts::rhs.currIndex = startIndex+1;
                        }
                        // someVar = 6 or someVar += 5 
                        else {
                            if (assignmentTokenIndex != startIndex+1) {
                                currentStartToken.ReportTokenError("!!!!!!!!!!!!!!!!!! ExtractAssignmentParts expected assignmentTokenIndex mismatch");
                                return false;
                            }
                            AssignmentParts::rhs.currIndex = startIndex+2;
                        }
                    }

                    return true;
                }
            }
        }
    }
}