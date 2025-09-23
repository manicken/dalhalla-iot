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

#include "HAL_JSON_SCRIPT_ENGINE_IfStatement.h"
#include "HAL_JSON_SCRIPT_ENGINE_StatementBlock.h"

#include <csignal>

namespace HAL_JSON {
    namespace ScriptEngine {

        BranchBlock::BranchBlock()
        {

        }

        BranchBlock::~BranchBlock()
        {
            delete[] items;
        }

        HALOperationResult BranchBlock::Exec()
        {
            for (int i=0;i<itemsCount;i++) {
                StatementBlock& statementItem = items[i];
                if (statementItem.handler == nullptr) {
                    printf("\nERRORERRORERRORERRORERRORERRORERRORERRORERRORERROR statementItem.handler == nullptr\n");
                    break;
                }
                HALOperationResult res = statementItem.handler(statementItem.context);
                if (res != HALOperationResult::Success) {
                    return res; // direct return on any failure here
                }
            }
            return HALOperationResult::Success;
        }

        ConditionalBranch::ConditionalBranch()
        {

        }
        ConditionalBranch::~ConditionalBranch()
        {
            if (deleter && context) {
                deleter(context);
                context = nullptr;
            }
            //delete[] items; is deleted by BranchBlock destructor

        }
        void ConditionalBranch::Set(Tokens& tokens)
        {
#if defined(ESP32) == false && defined(ESP8266) == false
            printf("(%d) ConditionalBranch::Set: %s\n", tokens.currIndex, tokens.Current().ToString().c_str());
#endif
            // consume the If / ElseIf token itself
            tokens.currIndex++;
            // set the slice to the items of this expression
            tokens.currentEndIndex = tokens.currIndex + tokens.Current().itemsInBlock;
            tokens.firstTokenStartOffset = nullptr;
            // the following consumes the expression tokens
            ExpressionTokens* expTokens = Expressions::GenerateRPNTokens(tokens); // note here. expTokens is non owned
            //if (expTokens == nullptr) {
            //    return;
            //}
#if defined(ESP32) == false && defined(ESP8266) == false
            printf("(%d) ConditionalBranch::Set after GenerateRPNTokens: %s\n", tokens.currIndex, tokens.Current().ToString().c_str());
#endif
            //printf("ConditionalBranch::Set expTokens:\n%s\n", PrintExpressionTokensOneRow(*expTokens, 0, expTokens->currentCount).c_str());
            // restore the slice to full token array
            tokens.currentEndIndex = tokens.count;
            // builds the temporary tree using memory pool
            LogicRPNNode* lrpnNode = Expressions::BuildLogicTree(expTokens); // note here. lrpnNode is non owned 

            if (lrpnNode->calcRPNStartIndex != -1) { // pure calc compare expression, no logic
                context = new CalcRPN(expTokens, lrpnNode->calcRPNStartIndex, lrpnNode->calcRPNEndIndex);
                handler = &LogicExecNode::Eval_Calc; // borrow this
                deleter = DeleteAs<CalcRPN>;
            } else {
                LogicExecNode* newExecNode = new LogicExecNode(expTokens, lrpnNode);
                context = newExecNode;
                deleter = DeleteAs<LogicExecNode>;
                handler = newExecNode->handler; // just copy this
            }

            //when consumed we are at the then
            Token& thenToken = tokens.GetNextAndConsume();//.items[tokens.currIndex++]; // get and consume
            if (thenToken.type != TokenType::Then) {
                ReportTokenError(thenToken, " !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ERROR: is not a then token: ");
                return;
            }
            // here extract the itemsCount
            itemsCount = thenToken.itemsInBlock;
#if defined(ESP32) == false && defined(ESP8266) == false
            printf("(%d) ---------------------------------------------------------------------------------------------- THEN token item count: (%d)\n",tokens.currIndex-1, itemsCount);
#endif
            items = new StatementBlock[itemsCount];

            for (int i=0;i<itemsCount;i++) {
                if (tokens.SkipIgnoresAndEndIf() == false) {
                    printf("SERIOUS ERROR - reached end\n");
                    break;
                }
                items[i].Set(tokens); // each call should consume all tokens
            }
        }

        UnconditionalBranch::UnconditionalBranch(Tokens& tokens)
        {
#if defined(ESP32) == false && defined(ESP8266) == false
            printf("(%d) UnconditionalBranch::UnconditionalBranch: %s\n", tokens.currIndex, tokens.Current().ToString().c_str());
#endif
            const Token& elseToken = tokens.GetNextAndConsume();//.items[tokens.currIndex++]; // get and consume

            itemsCount = elseToken.itemsInBlock;
            items = new StatementBlock[itemsCount];

            for (int i=0;i<itemsCount;i++) {
                if (tokens.SkipIgnoresAndEndIf() == false) {
                    printf("SERIOUS ERROR - reached end\n");
                    break;
                }
                items[i].Set(tokens);
            }
        }
        UnconditionalBranch::~UnconditionalBranch()
        {
            //delete[] items; is deleted by BranchBlock destructor
        }

        IfStatement::IfStatement(Tokens& tokens)
        {
            elseBranchFound = false;
            Token& ifToken = tokens.Current(); // this now points to the if-type token
            if (ifToken.type != TokenType::If) { printf("\nERROR ----- ifToken.type != TokenType::If\n");}
            branchItemsCount = ifToken.itemsInBlock;
            if (ifToken.hasElse == 1) branchItemsCount--; // minus one as the else case is handled separately
            //printf("\n----------------------------------------------------------------- branchItemsCount:%d\n",branchItemsCount);
            branchItems = new ConditionalBranch[branchItemsCount];
            // allways consume the first If condition
            branchItems[0].Set(tokens);

            // the following will ONLY go over ELSEIF ConditionalBranch:es
            for (int i=1;i<branchItemsCount;i++) {
                //printf("\n---------------------------- loading brachitem:%d\n",i);
                Token& token = tokens.Current();
                if (token.type != TokenType::ElseIf) {
                    printf("\n ERROR ----  TOKEN IS NOT A ELSEIF\n");
                    break;
                }
                // this will consume all tokens that actually belongs to this block
                branchItems[i].Set(tokens); 
            }
            Token& token = tokens.Current();
            if (token.type == TokenType::Else) {
#if defined(ESP32) == false && defined(ESP8266) == false
                printf("\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ found ELSE token @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
#endif
                // this will consume all tokens that actually belongs to this block
                elseBranch = new UnconditionalBranch(tokens);
                elseBranchFound = true;
            } 
            else {
#if defined(ESP32) == false && defined(ESP8266) == false
                printf("\n@???????????????????????????????????????????? found NOT ANY ELSE token @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
#endif
                elseBranch = nullptr;
                elseBranchFound = false;
            }
            /*if (tokens.currIndex == 85) {
                    raise(SIGTRAP); // triggers a breakpoint in GDB
                    
            }*/
        }
        IfStatement::~IfStatement()
        {
            delete[] branchItems;
            delete elseBranch;
        }

        HALOperationResult IfStatement::Handler(void* context) {
            if (context == nullptr) {
                printf("\n IfStatement::Handler ContextWasNullPtr\n");
                return HALOperationResult::ContextWasNullPtr;
            }
            IfStatement* ifStatement = static_cast<IfStatement*>(context);
            if (ifStatement == nullptr) {
                printf("\n IfStatement::Handler ifStatement was nullprtr\n");
                return HALOperationResult::ContextWasNullPtr;
            }
            int ifStatementBranchItemsCount = ifStatement->branchItemsCount;
            ConditionalBranch* ifStatementBranchItems = ifStatement->branchItems;
            for (int i=0;i<ifStatementBranchItemsCount;i++) {
                HALOperationResult res = ifStatementBranchItems[i].handler(ifStatementBranchItems[i].context);
                if (res == HALOperationResult::IfConditionTrue) {
                    return ifStatementBranchItems[i].Exec();
                } else if (res != HALOperationResult::IfConditionFalse) {
#if defined(ESP32) == false && defined(ESP8266) == false
                    printf("\n IfStatement::Handler - did execute a error \n");
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