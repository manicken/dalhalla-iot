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

#include "DALHAL_SCRIPT_ENGINE_IfStatement.h"

#include <DALHAL/Support/DALHAL_Logger.h>

#include <DALHAL/ScriptEngine/Runtime/DALHAL_SCRIPT_ENGINE_StatementBlock.h>
#include <DALHAL/ScriptEngine/Runtime/IfStatement/DALHAL_SCRIPT_ENGINE_BranchBlock.h>
#include <DALHAL/ScriptEngine/Runtime/IfStatement/DALHAL_SCRIPT_ENGINE_ConditionalBranch.h>
#include <DALHAL/ScriptEngine/Runtime/IfStatement/DALHAL_SCRIPT_ENGINE_UnconditionalBranch.h>
#include <DALHAL/ScriptEngine/Parser/DALHAL_SCRIPT_ENGINE_Script_Token.h>

#include <csignal>

namespace DALHAL {
    namespace ScriptEngine {

        IfStatement::IfStatement() : branchItems(nullptr), branchItemsCount(0), elseBranch(nullptr) { }

        bool IfStatement::Set(ScriptTokens& tokens) {
            //elseBranchFound = false;
            ScriptToken& ifToken = tokens.Current(); // this now points to the if-type token
            if (ifToken.type != ScriptTokenType::If) { 
                ifToken.ReportTokenError(F("LOAD ERROR ----- ifToken.type != TokenType::If\n"));
                return false; // stop loading script
            }
            branchItemsCount = ifToken.itemsInBlock;
            if (ifToken.hasElse == 1) branchItemsCount--; // minus one as the else case is handled separately
            //printf("\n----------------------------------------------------------------- branchItemsCount:%d\n",branchItemsCount);
            branchItems = new ConditionalBranch[branchItemsCount];
            // allways consume the first If condition
            branchItems[0].Set(tokens);

            // the following will ONLY go over ELSEIF ConditionalBranch:es
            for (int i=1;i<branchItemsCount;i++) {
                //printf("\n---------------------------- loading brachitem:%d\n",i);
                ScriptToken& token = tokens.Current();
                if (token.type != ScriptTokenType::ElseIf) {
                    token.ReportTokenError(F("LOAD ERROR ----  TOKEN IS NOT A ELSEIF\n"));
                    return false; // stop loading script
                }
                // this will consume all tokens that actually belongs to this block
                if (branchItems[i].Set(tokens) == false) {
                    return false; // stop loading script
                } 
            }
            ScriptToken& token = tokens.Current();
            if (token.type == ScriptTokenType::Else) {
#if defined(ESP32) == false && defined(ESP8266) == false
                printf("\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ found ELSE token @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
#endif
                // this will consume all tokens that actually belongs to this block
                elseBranch = new UnconditionalBranch();
                //elseBranchFound = true;
                if (elseBranch->Set(tokens) == false) {
                    return false;  // stop loading script
                }
                
            } 
            else {
#if defined(ESP32) == false && defined(ESP8266) == false
                printf("\n@???????????????????????????????????????????? found NOT ANY ELSE token @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
#endif
                elseBranch = nullptr;
                //elseBranchFound = false;
            }
            /*if (tokens.currIndex == 85) {
                    raise(SIGTRAP); // triggers a breakpoint in GDB
                    
            }*/
           return true;
        }

        IfStatement::~IfStatement()
        {
            delete[] branchItems;
            delete elseBranch;
        }

        HALOperationResult IfStatement::Handler(void* context) {
            if (context == nullptr) {
                GlobalLogger.Error(F("\n IfStatement::Handler ContextWasNullPtr\n"));
                return HALOperationResult::ContextWasNullPtr;
            }
            IfStatement* ifStatement = static_cast<IfStatement*>(context);
            /* no point doing this check
            if (ifStatement == nullptr) {
               GlobalLogger.Error(F("\n IfStatement::Handler ifStatement was nullprtr\n"));
                return HALOperationResult::ContextWasNullPtr;
            }*/
            int ifStatementBranchItemsCount = ifStatement->branchItemsCount;
            ConditionalBranch* ifStatementBranchItems = ifStatement->branchItems;
            for (int i=0;i<ifStatementBranchItemsCount;i++) {
                HALOperationResult res = ifStatementBranchItems[i].handler(ifStatementBranchItems[i].context);
                if (res == HALOperationResult::IfConditionTrue) {
                    return ifStatementBranchItems[i].Exec();
                } else if (res != HALOperationResult::IfConditionFalse) {
#if defined(ESP32) == false && defined(ESP8266) == false
                    printf("\n IfStatement::Handler - did return a error \n");
#endif
                    return res; // direct fail stop exec here??
                }
            }
            
            if (ifStatement->elseBranch) {
#if defined(ESP32) == false && defined(ESP8266) == false
                printf("\n IfStatement::Handler - else EXEC \n");
#endif
                return ifStatement->elseBranch->Exec();
            }
            // allways return success when all execution was a success
            return HALOperationResult::Success;
        }
    }
}