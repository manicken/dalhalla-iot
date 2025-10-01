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

#include "HAL_JSON_SCRIPT_ENGINE_Parser_Conditions.h"
#include "HAL_JSON_SCRIPT_ENGINE_Expression_Parser.h"
#include "HAL_JSON_SCRIPT_ENGINE_Reports.h"

namespace HAL_JSON {
    namespace ScriptEngine {
        namespace Parser {
            namespace Conditions {
                int Count_IfTokens(ScriptTokens& _tokens) {
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

                bool VerifyBlocks(ScriptTokens& _tokens) {
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
                int CountConditionTokens(ScriptTokens& _tokens, int start) {
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
                
                bool MergeConditions(ScriptTokens& _tokens) {
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
                bool VerifyConditionBlocks(ScriptTokens& _tokens) {
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
                bool EnsureBlocksContainItems(ScriptTokens& _tokens) {
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
                // this version do work but is ineffective as it basically scans
                // all tokens over and over again
                void CountBlockItems(ScriptTokens& _tokens) {
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
            }
        }
    }
}