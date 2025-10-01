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

#include "HAL_JSON_SCRIPT_ENGINE_Parser.h"

#include "HAL_JSON_SCRIPT_ENGINE_Tokenizer.h"

namespace HAL_JSON {
    namespace ScriptEngine {

        AssignmentParts g_assignmentParts;  // single reusable instance

        static const ScriptTokenType actionStartTypes[] = {ScriptTokenType::ActionSeparator, ScriptTokenType::And, ScriptTokenType::Else, ScriptTokenType::EndIf, ScriptTokenType::Then, ScriptTokenType::NotSet};
        static const ScriptTokenType actionEndTypes[] =   {ScriptTokenType::ActionSeparator, ScriptTokenType::And, ScriptTokenType::Else, ScriptTokenType::EndIf, ScriptTokenType::If, ScriptTokenType::ElseIf, ScriptTokenType::EndOn, ScriptTokenType::NotSet};
        
        void Parser::ReportError(const char* msg) {
    #if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
            std::cout << "Error: " << msg << std::endl;
    #else
            GlobalLogger.Error(F("Rule Set Parse:"), msg);
    #endif
        }
#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__) || defined(DEBUG_PRINT_SCRIPT_ENGINE)
        void Parser::ReportInfo(std::string msg) {
            printf("\n%s\n", msg.c_str());
        }
   #define DBGSTR(x) x     
#else
#define ReportInfo(msg) ((void)0)
#define DBGSTR(x) ""
#endif

        int Parser::Count_IfTokens(ScriptTokens& _tokens) {
            ScriptToken* tokens = _tokens.items;
            int tokenCount = _tokens.count;
            int currLevel = 0;
            int maxLevel = 0;
            for (int i = 0; i < tokenCount; i++) {
                ScriptToken& token = tokens[i];
                if (token.type == ScriptTokenType::If) {
                    currLevel++;
                    if (currLevel > maxLevel) maxLevel = currLevel;
                }
                else if (token.type == ScriptTokenType::EndIf) {
                    currLevel--;
                }
            }
            return maxLevel;
        }

        bool Parser::VerifyBlocks(ScriptTokens& _tokens) {
            ScriptToken* tokens = _tokens.items;
            int tokenCount = _tokens.count;
            int onLevel = 0;
            int ifLevel = 0;
            int ifTokenCount = Count_IfTokens(_tokens);
            if (ifTokenCount <= 0) ifTokenCount = 1;
            ScriptToken** ifStack = new ScriptToken*[ifTokenCount]();
            int ifStackIndex = 0;
            ScriptToken* lastOn = nullptr;
            bool otherErrors = false;

            bool expecting_do_then = false;
            int last_If_or_On_Index = 0;

            for (int i = 0; i < tokenCount; i++) {
                ScriptToken& token = tokens[i];
                if (token.type == ScriptTokenType::If) {
                    ifLevel++;
                    ifStack[ifStackIndex++] = &token;
                    expecting_do_then = true;
                    last_If_or_On_Index = i;
                }
                else if (token.type == ScriptTokenType::ElseIf) {
                    expecting_do_then = true;
                    last_If_or_On_Index = i;

                    ScriptToken* currentIf = ifStack[ifStackIndex - 1]; // top of the stack

                    // check that no Else has already been found
                    if (currentIf->hasElse) {
                        token.ReportTokenError("'elseif' cannot appear after 'else'");
                        otherErrors = true;
                    }
                }
                else if (token.type == ScriptTokenType::Then) {
                    if (!expecting_do_then) {
                        token.ReportTokenError("'do/then' without preceding 'if' or 'on'");
                        otherErrors = true;
                    }
                    else if (last_If_or_On_Index + 1 == i) {
                        token.ReportTokenError("Missing condition between 'if/on/elseif' and 'then/do'");
                        otherErrors = true;
                    }
                    expecting_do_then = false;
                }
                else if (token.type == ScriptTokenType::EndIf) {
                    if (ifLevel == 0) {
                        token.ReportTokenError("'endif' without matching 'if'");
                        otherErrors = true;
                    }
                    else {
                        if (ifStackIndex > 0)
                            ifStackIndex--;
                        ifLevel--;
                    }

                    if (expecting_do_then) {
                        token.ReportTokenError("missing 'do/then' after last 'if'");
                        otherErrors = true;
                    }
                }
                else if (token.type == ScriptTokenType::Else) {
                    ScriptToken* currentIf = ifStack[ifStackIndex - 1]; // top of the stack

                    // check for multiple Else
                    if (currentIf->hasElse) {
                        token.ReportTokenError("Multiple 'else' blocks in the same 'if'");
                        otherErrors = true;
                    } else {
                        currentIf->hasElse = 1;
                    }
                } 
                else if (token.type == ScriptTokenType::On) {
                    if (ifLevel != 0 || onLevel != 0) {
                        token.ReportTokenError("'on' block cannot be nested");
                        otherErrors = true;
                    } else {
                        lastOn = &token;
                        onLevel++;
                        expecting_do_then = true;
                        last_If_or_On_Index = i;
                    }
                }
                else if (token.type == ScriptTokenType::EndOn) {
                    if (onLevel == 0) {
                        token.ReportTokenError("'endon' without matching 'on'");
                        otherErrors = true;
                    } else
                        onLevel--;

                    if (expecting_do_then) {
                        token.ReportTokenError("missing 'do' after last 'on'");
                        otherErrors = true;
                    }
                }
                else if (onLevel == 0 && ifLevel == 0) {
                    token.ReportTokenError("action/assigment expression tokens cannot be outside root blocks");
                    otherErrors = true;
                }
            }

            if (ifLevel != 0) {
                for (int i=0;i<ifStackIndex;i++) { // only print last 'errors'
                    ifStack[i]->ReportTokenError("Unmatched 'if' block");
                }
                
            }
            if (onLevel != 0) {
                if (lastOn)
                    lastOn->ReportTokenError("Unmatched 'on' block: ");
                else
                    ReportError("Unmatched 'on' block: <null>");
            }
            delete[] ifStack;
            return (ifLevel == 0) && (onLevel == 0) && (otherErrors == false);
        }
        int Parser::CountConditionTokens(ScriptTokens& _tokens, int start) {
            ScriptToken* tokens = _tokens.items;
            int tokenCount = _tokens.count;
            int count = 0;
            for (int i = start; i < tokenCount; i++) {
                if (tokens[i].type == ScriptTokenType::Then) {
                    return count;
                }
                else
                    count++;
            }
            return -1; // mean we did not find the do or then token
        }
        
        bool Parser::MergeConditions(ScriptTokens& _tokens) {
            ScriptToken* tokens = _tokens.items;
            int tokenCount = _tokens.count;
            for (int i = 0; i < tokenCount; i++) {
                if (((tokens[i].type == ScriptTokenType::If) || (tokens[i].type == ScriptTokenType::ElseIf)) == false) continue;
                i++;
                int conditionTokenCount = CountConditionTokens(_tokens, i);
#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)   
                std::cout << "If case token count: " << conditionTokenCount << "\n";
#endif
                if (conditionTokenCount == -1) return false; // failsafe
                
                // if multiple tokens then there could be 'and' / 'or' keywords
                // replace them with && || respective
                // for uniform and easier condition parse
                if (conditionTokenCount > 1) {
                    
                    for (int j=i;j<i+conditionTokenCount;j++) {
                        if (tokens[j].type == ScriptTokenType::And) {
                            char* str = (char*)tokens[j].start; // need to change this text
                            str[0] = '&';
                            str[1] = '&';
                            str[2] = '\0';
                            tokens[j].end--;
                        } else if (tokens[j].type == ScriptTokenType::Or) {
                            char* str = (char*)tokens[j].start; // need to change this text
                            str[0] = '|';
                            str[1] = '|';
                        }
                    }
                    
                }

                tokens[i].MarkTokenGroup(conditionTokenCount, ScriptTokenType::IfCondition);
                i += conditionTokenCount; // skip all
            }
            return true;
        }

        bool Parser::MergeActions2(ScriptTokens& _tokens) {
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
// this version do work but is ineffective as it basically scans
// all tokens over and over again
        void Parser::CountBlockItems(ScriptTokens& _tokens) {
            ScriptToken* tokens = _tokens.items;
            int tokenCount = _tokens.count;
            int rootLevelBlockCount = 0;
            int blockLevel = 0;

            for (int i = 0; i < tokenCount; ++i) {
                ScriptToken& token = tokens[i];

                // Track root-level blocks
                if (token.type == ScriptTokenType::If || token.type == ScriptTokenType::On) {
                    if (blockLevel == 0)
                        rootLevelBlockCount++;
                    blockLevel++;
                }
                else if (token.type == ScriptTokenType::EndIf || token.type == ScriptTokenType::EndOn) {
                    blockLevel--;
                }

                // Count branches in an If block: Then + ElseIf + optional Else
                if (token.type == ScriptTokenType::If) {
                    int branchCount = 1;  // initial Then branch
                    token.hasElse = 0;

                    for (int j = i + 1; j < tokenCount; ++j) {
                        ScriptToken& t = tokens[j];

                        // Break at parent EndIf
                        if (t.type == ScriptTokenType::EndIf) break;

                        // Count ElseIf or Else
                        if (t.type == ScriptTokenType::ElseIf || t.type == ScriptTokenType::Else) {
                            branchCount++;
                            if (t.type == ScriptTokenType::Else) {
                                token.hasElse = 1;
                            }
                        }

                        // Skip nested If blocks entirely
                        if (t.type == ScriptTokenType::If) {
                            int nestedLevel = 1;
                            for (++j; j < tokenCount && nestedLevel > 0; ++j) {
                                ScriptToken& nt = tokens[j];
                                if (nt.type == ScriptTokenType::If) nestedLevel++;
                                else if (nt.type == ScriptTokenType::EndIf) nestedLevel--;
                            }
                            --j; // adjust outer loop
                        }
                    }

                    token.itemsInBlock = branchCount;
                }
                // Count actions in Then, ElseIf, Else, or On blocks
                else if (token.type == ScriptTokenType::Then || token.type == ScriptTokenType::Else || 
                        token.type == ScriptTokenType::ElseIf || token.type == ScriptTokenType::On) { // just a remainder to myself here we do actually need to reqognize the 'on' type

                    int count = 0;
                    int nestedLevel = 0;
                    
                    for (int j = i + 1; j < tokenCount; ++j) {
                        ScriptToken& t = tokens[j];

                        if (t.type == ScriptTokenType::Ignore || t.type == ScriptTokenType::Merged) continue;
                        if (t.type == ScriptTokenType::And || t.type == ScriptTokenType::ActionSeparator) continue;

                        // Nested If counts as single statement in parent branch
                        if (t.type == ScriptTokenType::If) {
                            if (nestedLevel == 0) count++;
                            nestedLevel++;
                            continue;
                        }

                        // Track nested EndIf/EndOn
                        if (t.type == ScriptTokenType::EndIf || t.type == ScriptTokenType::EndOn) {
                            if (nestedLevel > 0) {
                                nestedLevel--;
                                continue;
                            } else {
                                break; // branch ends
                            }
                        }

                        // Stop at next sibling branch
                        if (nestedLevel == 0 && (t.type == ScriptTokenType::ElseIf || t.type == ScriptTokenType::Else)) {
                            break;
                        }

                        if (t.type == ScriptTokenType::Action && nestedLevel == 0) count++;
                    }

                    token.itemsInBlock = count;
                }
            }

            _tokens.rootBlockCount = rootLevelBlockCount;
        }
        
        bool Parser::EnsureActionBlocksContainItems(ScriptTokens& _tokens) {
            const ScriptToken* tokens = _tokens.items;
            int tokenCount = _tokens.count;
            bool anyError = false;
            for (int i = 0; i < tokenCount; ++i) {
                const ScriptToken& token = tokens[i];
                if (((token.type == ScriptTokenType::Then) || (token.type == ScriptTokenType::Else)) == false) continue;
                if (token.itemsInBlock == 0) {
                    token.ReportTokenError("EnsureActionBlocksContainItems - empty action(s) block detected");
                    anyError = true;
                }
            }
            return anyError == false;
        }

        bool Parser::VerifyConditionBlocks(ScriptTokens& _tokens) {
            ScriptToken* tokens = _tokens.items;
            int tokenCount = _tokens.count;
            bool anyError = false;
            for (int i = 0; i < tokenCount; ++i) {
                const ScriptToken& token = tokens[i];
                if (((token.type == ScriptTokenType::If) || (token.type == ScriptTokenType::ElseIf)) == false) continue;
                //const char* conditions = tokens[i+1].text;
                ScriptTokens conditions;
                conditions.items = &tokens[i+1];
                if (tokens[i+1].itemsInBlock != 0) {
                    conditions.count = tokens[i+1].itemsInBlock;
                } else {
                    conditions.count = 1;
                }
                ReportInfo("\n"); // newline
#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__) || defined(DEBUG_PRINT_SCRIPT_ENGINE)
                ReportInfo(conditions.ToString());
#endif
                ReportInfo("\n"); // newline
                conditions.currentEndIndex = conditions.count;
                if (Expressions::ValidateExpression(conditions, ExpressionContext::IfCondition) == false) anyError = true;

            }
            return anyError == false;
        }

        bool Parser::VerifyActionBlocks(ScriptTokens& _tokens) {
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

        bool Parser::ValidateParseScript(ScriptTokens& _tokens, bool validateOnly) {

            if (validateOnly) {
                ReportInfo("**********************************************************************************\n");
                ReportInfo("*                            RAW TOKEN LIST                                      *\n");
                ReportInfo("**********************************************************************************\n");
#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__) || defined(DEBUG_PRINT_SCRIPT_ENGINE)
                ReportInfo(PrintScriptTokens(_tokens,0) + "\n");
#endif
                ReportInfo("\n VerifyBlocks (BetterError): ");
                //MEASURE_TIME(" VerifyBlocks time: ",
                if (validateOnly && VerifyBlocks(_tokens) == false) {
                    ReportInfo("[FAIL]\n");
                    return false;
                }
                //);
                ReportInfo("[OK]\n");
            }
            // if here then we can safely parse all blocks
            //MEASURE_TIME("\n MergeConditions time: ",
            ReportInfo("\n MergeConditions: ");
            MergeConditions(_tokens);
            ReportInfo("[OK]\n");
            //);
            //MEASURE_TIME("\n MergeActions time: ",
            ReportInfo("\n MergeActions: ");
            MergeActions2(_tokens);
            ReportInfo("[OK]\n");
            //);
            //MEASURE_TIME("\n CountBlockItems time: ",
            ReportInfo("\n CountBlockItems: ");
            CountBlockItems(_tokens); // sets the metadata itemsInBlock
            ReportInfo("[OK]\n");
            //);
            ReportInfo("**********************************************************************************\n");
            ReportInfo("*                            PARSED TOKEN LIST                                   *\n");
            ReportInfo("**********************************************************************************\n");
#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__) || defined(DEBUG_PRINT_SCRIPT_ENGINE)
            ReportInfo(PrintScriptTokens(_tokens,0) + "\n");
#endif
            if (validateOnly) {
                //MEASURE_TIME("\n EnsureActionBlocksContainItems time: ",
                ReportInfo("\n EnsureActionBlocksContainItems: ");
                if (validateOnly && EnsureActionBlocksContainItems(_tokens) == false) { // uses the metadata itemsInBlock to determine if there are invalid
                    ReportInfo("[FAIL]\n");
                    return false;
                }
                ReportInfo("[OK]\n");
            //);
                
               // MEASURE_TIME("\n VerifyConditionBlocks time: ",
                ReportInfo("\n VerifyConditionBlocks: \n");
                if (VerifyConditionBlocks(_tokens) == false) {
                    ReportInfo("[FAIL @ VerifyConditionBlocks]\n");
                    return false;
                }
            //);
            //MEASURE_TIME("\n VerifyActionBlocks time: ",
                ReportInfo("\n VerifyActionBlocks: \n");
                if (VerifyActionBlocks(_tokens) == false) {
                    ReportInfo("[FAIL @ VerifyActionBlocks]\n");
                    return false;
                }
            //);
            }
            return true;
        }
#define USE_COMBINED_PARSE_TOKENS_FUNC

        bool Parser::ReadAndParseScriptFile(const char* filePath, void (*parsedOKcallback)(ScriptTokens& tokens)) {
            char* fileContents = nullptr;// = ReadFileToMutableBuffer(filePath, fileSize);
            MEASURE_TIME("ReadAndParseScriptFile - load_text_file time: ",
            LittleFS_ext::FileResult fileResult = LittleFS_ext::load_text_file(filePath, &fileContents);
            if (fileResult != LittleFS_ext::FileResult::Success) {
                ReportInfo("Error: file could not be read/or is empty\n");
                return false;
            }
        );
        /*MEASURE_TIME("FixNewLines time: ",
            // fix newlines so that they only consists of \n 
            // for easier parsing
            FixNewLines(fileContents);  // now obsolete as LittleFS_ext::load_text_file automatically normalizes newlines to \n
        );*/
#ifndef USE_COMBINED_PARSE_TOKENS_FUNC
            MEASURE_TIME("StripComments time: ",
            // replaces all comments with whitespace
            // make it much simpler to parse the contents 
            StripComments(fileContents); // not needed when integrated into ParseTokens
            );
#endif
            /* just some debug print
            char* ptr = fileContents;
            printf("\n");
            while (*ptr) {
                printf("%02X ", *ptr++);
            }
            printf("\n");
            */
            int tokenCount = 0;
           //MEASURE_TIME("CountTokens time: ",
#ifndef USE_COMBINED_PARSE_TOKENS_FUNC
            tokenCount = CountTokens(fileContents);
#else
            tokenCount = ParseAndTokenize<ScriptToken>(fileContents, nullptr, -1); // count in the same function
#endif
           //);
            ReportInfo("Token count: " + std::to_string(tokenCount) + "\n");
            ScriptTokens tokens(tokenCount);
            
            //MEASURE_TIME("Tokenize time: ",

#ifndef USE_COMBINED_PARSE_TOKENS_FUNC
            if (TokenizeScript(fileContents, tokens) == false)
#else
            if (ParseAndTokenize(fileContents, tokens.items, tokenCount) == -1)
#endif
            {
                ReportInfo("Error: could not Tokenize\n");
                delete[] fileContents;
                return false;
            }
            //);
            bool anyError = false;
            MEASURE_TIME("ValidateParseScript time: ",
            
            if (ValidateParseScript(tokens, parsedOKcallback==nullptr)) {
                ReportInfo("ParseScript [OK]\n");
                
                if (parsedOKcallback) {
                    MEASURE_TIME("loadscript time: ",
                    parsedOKcallback(tokens);
                    );
                }
                    
                
            } else {
                ReportInfo("ParseScript [FAIL]\n");
                anyError = true;
            }
            );
            // dont forget to free/delete
            delete[] fileContents;
            return anyError == false;
        }

        

        AssignmentParts* Parser::ExtractAssignmentParts(ScriptTokens& tokens) {
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



        bool Parser::ParseExpressionTest(const char* filePath) {
            
            char* fileContents = nullptr;// = ReadFileToMutableBuffer(filePath, fileSize);
            LittleFS_ext::FileResult fileResult = LittleFS_ext::load_text_file(filePath, &fileContents);
            if (fileResult != LittleFS_ext::FileResult::Success) {
                ReportInfo("Error: file could not be read/or is empty\n");
                return false;
            }

            /*MEASURE_TIME("FixNewLines and StripComments time: ",
            // fix newlines so that they only consists of \n 
            // for easier parsing
            //FixNewLines(fileContents); // now obsolete as LittleFS_ext::load_text_file automatically normalizes newlines to \n
            // replaces all comments with whitespace
            // make it much simpler to parse the contents 
            StripComments(fileContents);
            );
*/
            //int tokenCount = CountTokens(fileContents);
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
            

            ReportInfo(DBGSTR("**********************************************************************************\n"));
            ReportInfo(DBGSTR("*                            PARSED TOKEN LIST                                   *\n"));
            ReportInfo(DBGSTR("**********************************************************************************\n"));

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
            if (Expressions::ValidateExpression(tokens, ExpressionContext::IfCondition) == false)
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

        bool Parser::ParseActionExpressionTest(const char* filePath) {
            
            char* fileContents = nullptr;
            LittleFS_ext::FileResult fileResult = LittleFS_ext::load_text_file(filePath, &fileContents);
            if (fileResult != LittleFS_ext::FileResult::Success) {
                ReportInfo("Error: file could not be read/or is empty\n");
                return false;
            }
/*
            MEASURE_TIME("FixNewLines and StripComments time: ",
            // fix newlines so that they only consists of \n 
            // for easier parsing
            //FixNewLines(fileContents);  // now obsolete as LittleFS_ext::load_text_file automatically normalizes newlines to \n
            // replaces all comments with whitespace
            // make it much simpler to parse the contents 
            StripComments(fileContents);
            );
*/
            //int tokenCount = CountTokens(fileContents);
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
            AssignmentParts* action = ExtractAssignmentParts(tokens);

            ReportInfo("**********************************************************************************\n");
            ReportInfo("*                            VALIDATE PARSED TOKEN LIST                          *\n");
            ReportInfo("**********************************************************************************\n");
            // need to be done before any ValidateExpression
            // and that normally mean before any validation first occur
            // i.e if many script files are to be validated this need to happen before any of that happens
            Expressions::CalcStackSizesInit();
            if (Expressions::ValidateExpression(action->rhs, ExpressionContext::Assignment) == false)
            {
                ReportInfo("Error: validate tokens fail\n");
                delete[] fileContents;
                return false;
            }
            Expressions::PrintCalcedStackSizes();
            Expressions::InitStacks();

            ReportInfo("\nAction lhs:" + action->lhs.ToString() + "\n");
            ReportInfo("Action assigment operator:" + std::string(1, action->op) + "\n\n");

            ExpressionTokens* newDirect = Expressions::GenerateRPNTokens(action->rhs);
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