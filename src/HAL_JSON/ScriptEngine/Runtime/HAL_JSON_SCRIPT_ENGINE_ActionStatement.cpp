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

#include "HAL_JSON_SCRIPT_ENGINE_ActionStatement.h"

#include "HAL_JSON_SCRIPT_ENGINE_Operators.h"

#include "../Parser/HAL_JSON_SCRIPT_ENGINE_Parser_Expressions.h"
#include "../Parser/HAL_JSON_SCRIPT_ENGINE_Parser_Actions.h"
#include "../HAL_JSON_SCRIPT_ENGINE_Reports.h"


namespace HAL_JSON {
    namespace ScriptEngine {

        ActionStatement::ActionStatement(ScriptTokens& tokens, ActionHandler& handlerOut)
        {
            // ExtractAssignmentParts "consumes" the tokens until the next action or whatever coming after
            // just run this no checking here as at this stage the script is valid
            Parser::Actions::ExtractAssignmentParts(tokens); 
            ReportInfo("\n**************** Action ************* lefthand side:" + Parser::Actions::AssignmentParts::lhs.ToString() + "\n");
            ReportInfo("**************** Action ******** assigment operator:" + std::string(1, Parser::Actions::AssignmentParts::op) + "\n");
            
            ZeroCopyString varOperand = Parser::Actions::AssignmentParts::lhs;
            ZeroCopyString funcName = Parser::Actions::AssignmentParts::lhs;
            varOperand = funcName.SplitOffHead('#');
            target = new CachedDeviceAccess(varOperand, funcName);

            if (Expressions::IsExpressionEmpty(Parser::Actions::AssignmentParts::rhs)) {
                calcRpn = nullptr; // not used here
            } else {
                ExpressionTokens* expTokens = Expressions::GenerateRPNTokens(Parser::Actions::AssignmentParts::rhs); // note here. expTokens is non owned

                ReportInfo("\n***************** Action *********** rhs calc RPN: [");
                for (int i=0;i<expTokens->currentCount;i++) { // currentCount is set by GenerateRPNTokens and defines the current 'size'
                    ReportInfo(expTokens->items[i].ToString() + " ");
                }
                ReportInfo("]\n\n");

                calcRpn = new CalcRPN(expTokens, 0, expTokens->currentCount);
            }
            
            handlerOut = GetFunctionHandler(Parser::Actions::AssignmentParts::op);
        }
        ActionStatement::~ActionStatement()
        {
            delete target;
            delete calcRpn;
        }
        
        HALOperationResult ActionStatement::Assign_Handler(void* context) {
            ActionStatement* actionItem = static_cast<ActionStatement*>(context);
            HALOperationResult res = actionItem->calcRpn->DoCalc();
            if (res != HALOperationResult::Success) return res;
            HALValue val2write;
            if (halValueStack.GetFinalResult(val2write) == false) return HALOperationResult::ResultGetFail;
            return actionItem->target->WriteSimple(val2write);
        }
        HALOperationResult ActionStatement::Exec_Handler(void* context) {
            ActionStatement* actionItem = static_cast<ActionStatement*>(context);
            return actionItem->target->Exec();
        }
        /*
        HALOperationResult ActionStatement::AddAndAssign_Handler(void* context) {
            ActionStatement* actionItem = static_cast<ActionStatement*>(context);
            HALOperationResult res = actionItem->calcRpn->DoCalc();
            if (res != HALOperationResult::Success) return res;
            HALValue val2write;
            if (halValueStack.GetFinalResult(val2write) == false) return HALOperationResult::ResultGetFail;
            HALValue readVal;
            res = actionItem->target->ReadSimple(readVal);
            if (res != HALOperationResult::Success) return res;
            val2write = val2write + readVal;
            return actionItem->target->WriteSimple(val2write);
        }
        HALOperationResult ActionStatement::SubtractAndAssign_Handler(void* context) {
            ActionStatement* actionItem = static_cast<ActionStatement*>(context);
            HALOperationResult res = actionItem->calcRpn->DoCalc();
            if (res != HALOperationResult::Success) return res;
            HALValue val2write;
            if (halValueStack.GetFinalResult(val2write) == false) return HALOperationResult::ResultGetFail;
            HALValue readVal;
            res = actionItem->target->ReadSimple(readVal);
            if (res != HALOperationResult::Success) return res;
            val2write = readVal - val2write;
            return actionItem->target->WriteSimple(val2write);
        }
        HALOperationResult ActionStatement::MultiplyAndAssign_Handler(void* context) {
            ActionStatement* actionItem = static_cast<ActionStatement*>(context);
            HALOperationResult res = actionItem->calcRpn->DoCalc();
            if (res != HALOperationResult::Success) return res;
            HALValue val2write;
            if (halValueStack.GetFinalResult(val2write) == false) return HALOperationResult::ResultGetFail;
            HALValue readVal;
            res = actionItem->target->ReadSimple(readVal);
            if (res != HALOperationResult::Success) return res;
            val2write = readVal * val2write;
            return actionItem->target->WriteSimple(val2write);
        }
        HALOperationResult ActionStatement::DivideAndAssign_Handler(void* context) {
            ActionStatement* actionItem = static_cast<ActionStatement*>(context);
            HALOperationResult res = actionItem->calcRpn->DoCalc();
            if (res != HALOperationResult::Success) return res;
            HALValue val2write;
            if (halValueStack.GetFinalResult(val2write) == false) return HALOperationResult::ResultGetFail;
            HALValue readVal;
            res = actionItem->target->ReadSimple(readVal);
            if (res != HALOperationResult::Success) return res;
            val2write = readVal / val2write;
            return actionItem->target->WriteSimple(val2write);
        }
        HALOperationResult ActionStatement::ModulusAssign_Handler(void* context) {
            ActionStatement* actionItem = static_cast<ActionStatement*>(context);
            HALOperationResult res = actionItem->calcRpn->DoCalc();
            if (res != HALOperationResult::Success) return res;
            HALValue val2write;
            if (halValueStack.GetFinalResult(val2write) == false) return HALOperationResult::ResultGetFail;
            HALValue readVal;
            res = actionItem->target->ReadSimple(readVal);
            if (res != HALOperationResult::Success) return res;
            val2write = readVal & val2write;
            return actionItem->target->WriteSimple(val2write);
        }
        HALOperationResult ActionStatement::BitwiseOrAssign_Handler(void* context) {
            ActionStatement* actionItem = static_cast<ActionStatement*>(context);
            HALOperationResult res = actionItem->calcRpn->DoCalc();
            if (res != HALOperationResult::Success) return res;
            HALValue val2write;
            if (halValueStack.GetFinalResult(val2write) == false) return HALOperationResult::ResultGetFail;
            HALValue readVal;
            res = actionItem->target->ReadSimple(readVal);
            if (res != HALOperationResult::Success) return res;
            val2write = readVal | val2write;
            return actionItem->target->WriteSimple(val2write);
        }
        HALOperationResult ActionStatement::BitwiseAndAssign_Handler(void* context) {
            ActionStatement* actionItem = static_cast<ActionStatement*>(context);
            HALOperationResult res = actionItem->calcRpn->DoCalc();
            if (res != HALOperationResult::Success) return res;
            HALValue val2write;
            if (halValueStack.GetFinalResult(val2write) == false) return HALOperationResult::ResultGetFail;
            HALValue readVal;
            res = actionItem->target->ReadSimple(readVal);
            if (res != HALOperationResult::Success) return res;
            val2write = readVal & val2write;
            return actionItem->target->WriteSimple(val2write);
        }
        HALOperationResult ActionStatement::BitwiseExOrAssign_Handler(void* context) {
            ActionStatement* actionItem = static_cast<ActionStatement*>(context);
            HALOperationResult res = actionItem->calcRpn->DoCalc();
            if (res != HALOperationResult::Success) return res;
            HALValue val2write;
            if (halValueStack.GetFinalResult(val2write) == false) return HALOperationResult::ResultGetFail;
            HALValue readVal;
            res = actionItem->target->ReadSimple(readVal);
            if (res != HALOperationResult::Success) return res;
            val2write = readVal ^ val2write;
            return actionItem->target->WriteSimple(val2write);
        }
        HALOperationResult ActionStatement::BitwiseShiftRightAssign_Handler(void* context) {
            ActionStatement* actionItem = static_cast<ActionStatement*>(context);
            HALOperationResult res = actionItem->calcRpn->DoCalc();
            if (res != HALOperationResult::Success) return res;
            HALValue val2write;
            if (halValueStack.GetFinalResult(val2write) == false) return HALOperationResult::ResultGetFail;
            HALValue readVal;
            res = actionItem->target->ReadSimple(readVal);
            if (res != HALOperationResult::Success) return res;
            val2write = readVal >> val2write;
            return actionItem->target->WriteSimple(val2write);
        }
        HALOperationResult ActionStatement::BitwiseShiftLeftAssign_Handler(void* context) {
            ActionStatement* actionItem = static_cast<ActionStatement*>(context);
            HALOperationResult res = actionItem->calcRpn->DoCalc();
            if (res != HALOperationResult::Success) return res;
            HALValue val2write;
            if (halValueStack.GetFinalResult(val2write) == false) return HALOperationResult::ResultGetFail;
            HALValue readVal;
            res = actionItem->target->ReadSimple(readVal);
            if (res != HALOperationResult::Success) return res;
            val2write = readVal << val2write;
            return actionItem->target->WriteSimple(val2write);
        }

        ActionHandler ActionStatement::GetFunctionHandler(const char c) {
            if (c == '=') return &Assign_Handler;
            else if (c == '+') return &AddAndAssign_Handler;
            else if (c == '-') return &SubtractAndAssign_Handler;
            else if (c == '*') return &MultiplyAndAssign_Handler;
            else if (c == '/') return &DivideAndAssign_Handler;
            else if (c == '%') return &ModulusAssign_Handler;
            else if (c == '|') return &BitwiseOrAssign_Handler;
            else if (c == '&') return &BitwiseAndAssign_Handler;
            else if (c == '^') return &BitwiseExOrAssign_Handler;
            else if (c == '<') return &BitwiseShiftLeftAssign_Handler;
            else if (c == '>') return &BitwiseShiftRightAssign_Handler;
            else return nullptr; // should never happend
        }*/

        ActionHandler ActionStatement::GetFunctionHandler(const char c) {
            if (c == '=') return &Assign_Handler;
            else if (c == '+') return &CompoundAssign_Handler<OpAdd>;
            else if (c == '-') return &CompoundAssign_Handler<OpSub>;
            else if (c == '*') return &CompoundAssign_Handler<OpMul>;
            else if (c == '/') return &CompoundAssign_Handler<OpDiv>;
            else if (c == '%') return &CompoundAssign_Handler<OpMod>;
            else if (c == '|') return &CompoundAssign_Handler<OpBitOr>;
            else if (c == '&') return &CompoundAssign_Handler<OpBitAnd>;
            else if (c == '^') return &CompoundAssign_Handler<OpBitExOr>;
            else if (c == '<') return &CompoundAssign_Handler<OpBitLshift>;
            else if (c == '>') return &CompoundAssign_Handler<OpBitRshift>;
            else if (c == '(') return &Exec_Handler;
            else return nullptr; // should never happend
        }
    }
}