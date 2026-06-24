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

#include "DALHAL_SCRIPT_ENGINE_ConditionalBranch.h"

#include <DALHAL/ScriptEngine/Parser/DALHAL_SCRIPT_ENGINE_Script_Token.h>
#include <DALHAL/ScriptEngine/Parser/DALHAL_SCRIPT_ENGINE_Expression_Token.h>
#include <DALHAL/ScriptEngine/Parser/DALHAL_SCRIPT_ENGINE_Parser_Expressions.h>
#include <DALHAL/ScriptEngine/Runtime/DALHAL_SCRIPT_ENGINE_CalcRPN.h>
#include <DALHAL/ScriptEngine/Runtime/DALHAL_SCRIPT_ENGINE_StatementBlock.h>

#include <DALHAL/Support/DALHAL_Logger.h>

namespace DALHAL {
    namespace ScriptEngine {

        ConditionalBranch::ConditionalBranch() : BranchBlock(), context(nullptr), deleter(nullptr), handler(nullptr) { }

        ConditionalBranch::~ConditionalBranch()
        {
            if (deleter && context) {
                deleter(context);
                context = nullptr;
            }
            //delete[] items; is deleted by BranchBlock destructor

        }
        bool ConditionalBranch::Set(ScriptTokens& tokens)
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
            ScriptToken& thenToken = tokens.GetNextAndConsume();//.items[tokens.currIndex++]; // get and consume
            if (thenToken.type != ScriptTokenType::Then) {
                thenToken.ReportTokenError(F("LOAD ERROR - following is not a then token: "));
                return false;
            }
            // here extract the itemsCount
            itemsCount = thenToken.itemsInBlock;
#if defined(ESP32) == false && defined(ESP8266) == false
            printf("(%d) ---------------------------------------------------------------------------------------------- THEN token item count: (%d)\n",tokens.currIndex-1, itemsCount);
#endif
            items = new StatementBlock[itemsCount];

            for (int i=0;i<itemsCount;i++) {
                if (tokens.SkipIgnoresAndEndIf() == SkipTokenResult::ReachedEnd) {
                    GlobalLogger.Error(F("LOAD ERROR - reached end\n"));
                    return false;
                }
                items[i].Set(tokens); // each call should consume all tokens
            }
            // allways consume ignore/endif tokens
            if (tokens.SkipIgnoresAndEndIf() == SkipTokenResult::ReachedEnd) {
                GlobalLogger.Error(F("LOAD ERROR - reached end\n"));
                return false;
            }
            return true;
        }
    }
}