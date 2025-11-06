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

#include "HAL_JSON_SCRIPT_ENGINE_LogicExecNode.h"
#include "HAL_JSON_SCRIPT_ENGINE_CalcRPN.h"

//#define DEBUG_LOGIC_EXEC

namespace HAL_JSON {
    namespace ScriptEngine {

        LogicExecNode::LogicExecNode(ExpressionTokens* expTokens, LogicRPNNode* logicRPNnode) {
            LogicExecNode::GetSetHandlers(*logicRPNnode, handler, deleter);
            LogicRPNNode* childA_node = logicRPNnode->childA;
            LogicRPNNode* childB_node = logicRPNnode->childB;
            if (childA_node->calcRPNStartIndex != -1) { // calc node
                childA = new CalcRPN(expTokens, childA_node->calcRPNStartIndex, childA_node->calcRPNEndIndex);
            } else { // logic node
                childA = new LogicExecNode(expTokens, childA_node);
            }
            if (childB_node->calcRPNStartIndex != -1) { // calc node
                childB = new CalcRPN(expTokens, childB_node->calcRPNStartIndex, childB_node->calcRPNEndIndex);
            } else { // logic node
                childB = new LogicExecNode(expTokens, childB_node);
            }
        }

        HALOperationResult LogicExecNode::Eval_Calc(void* context) { // this is when root node is calc compare only
            CalcRPN* calcRpn = static_cast<CalcRPN*>(context);
            if (calcRpn == nullptr) {
                printf("\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!   CALC RPN WAS NULLPTR\n");
                return HALOperationResult::ContextWasNullPtr;
            }
            HALOperationResult res = calcRpn->DoCalc();
            if (res != HALOperationResult::Success) return res;
            HALValue val;
            if (halValueStack.GetFinalResult(val) == false) return HALOperationResult::ResultGetFail;
            if (val.asUInt() == 0) return HALOperationResult::IfConditionFalse;
            return HALOperationResult::IfConditionTrue;
        }

        HALOperationResult LogicExecNode::EvalAnd_LL(void* context) {  // LL (logic logic)
#ifdef DEBUG_LOGIC_EXEC
            printf("\nEvalAnd_LL\n");
#endif
            LogicExecNode* logicExecNode = static_cast<LogicExecNode*>(context);
            // evaluate left side
            LogicExecNode* logicExecNodeA = static_cast<LogicExecNode*>(logicExecNode->childA);
            HALOperationResult resA = logicExecNodeA->handler(logicExecNodeA);
            // short-circuit: if A is not true, return immediately the result (either False or any error)
            if (resA != HALOperationResult::IfConditionTrue) return resA;
            // only if A was true, evaluate B and return its result
            LogicExecNode* logicExecNodeB = static_cast<LogicExecNode*>(logicExecNode->childB);
            return logicExecNodeB->handler(logicExecNodeB);
        }
        HALOperationResult LogicExecNode::EvalAnd_LC(void* context) { // LC (logic calc)
#ifdef DEBUG_LOGIC_EXEC
            printf("\nEvalAnd_LC\n");
#endif
            LogicExecNode* logicExecNode = static_cast<LogicExecNode*>(context);
            // evaluate left side
            LogicExecNode* logicExecNodeA = static_cast<LogicExecNode*>(logicExecNode->childA);
            HALOperationResult resA = logicExecNodeA->handler(logicExecNodeA);
            // short-circuit: if A is not true, return immediately the result (either False or any error)
            if (resA != HALOperationResult::IfConditionTrue) return resA;
            // only if A was true, evaluate B and return its result
            return Eval_Calc(logicExecNode->childB);
        }
        HALOperationResult LogicExecNode::EvalAnd_CL(void* context) { // CL (calc logic)
#ifdef DEBUG_LOGIC_EXEC
            printf("\nEvalAnd_CL\n");
#endif
            LogicExecNode* logicExecNode = static_cast<LogicExecNode*>(context);
            // evaluate left side
            HALOperationResult resA = Eval_Calc(logicExecNode->childA);
            // short-circuit: if A is not true, return immediately the result (either False or any error)
            if (resA != HALOperationResult::IfConditionTrue) return resA;
            LogicExecNode* logicExecNodeB = static_cast<LogicExecNode*>(logicExecNode->childB);
            // only if A was true, evaluate B and return its result
            return logicExecNodeB->handler(logicExecNodeB);
        }
        HALOperationResult LogicExecNode::EvalAnd_CC(void* context) { // CC (calc calc)
#ifdef DEBUG_LOGIC_EXEC
            printf("\nEvalAnd_CC\n");
#endif
            LogicExecNode* logicExecNode = static_cast<LogicExecNode*>(context);
            // evaluate left side
            HALOperationResult resA = Eval_Calc(logicExecNode->childA);
            // short-circuit: if A is not true, return immediately the result (either False or any error)
            if (resA != HALOperationResult::IfConditionTrue) return resA;
            // only if A was true, evaluate B and return its result
            return Eval_Calc(logicExecNode->childB);
        }

        HALOperationResult LogicExecNode::EvalOr_LL(void* context) { // LL
#ifdef DEBUG_LOGIC_EXEC
            printf("\nEvalOr_LL\n");
#endif
            LogicExecNode* logicExecNode = static_cast<LogicExecNode*>(context);
            // evaluate left side
            LogicExecNode* logicExecNodeA = static_cast<LogicExecNode*>(logicExecNode->childA);
            HALOperationResult resA = logicExecNodeA->handler(logicExecNodeA);
            // short-circuit: if A is not false, return immediately the result (either True or any error)
            if (resA != HALOperationResult::IfConditionFalse)
                return resA;
            // only if A was false, evaluate B and return its result
            LogicExecNode* logicExecNodeB = static_cast<LogicExecNode*>(logicExecNode->childB);
            return logicExecNodeB->handler(logicExecNodeB);
        }
        HALOperationResult LogicExecNode::EvalOr_LC(void* context) { // LC
#ifdef DEBUG_LOGIC_EXEC
            printf("\nEvalOr_LC\n");
#endif
            LogicExecNode* logicExecNode = static_cast<LogicExecNode*>(context);
            // evaluate left side
            LogicExecNode* logicExecNodeA = static_cast<LogicExecNode*>(logicExecNode->childA);
            HALOperationResult resA = logicExecNodeA->handler(logicExecNodeA);
            // short-circuit: if A is not false, return immediately the result (either True or any error)
            if (resA != HALOperationResult::IfConditionFalse)
                return resA;
            // only if A was false, evaluate B and return its result
            return Eval_Calc(logicExecNode->childB);
        }
        HALOperationResult LogicExecNode::EvalOr_CL(void* context) { // CL
#ifdef DEBUG_LOGIC_EXEC
            printf("\nEvalOr_CL\n");
#endif
            LogicExecNode* logicExecNode = static_cast<LogicExecNode*>(context);
            HALOperationResult resA = Eval_Calc(logicExecNode->childA);
            // short-circuit: if A is not false, return immediately the result (either True or any error)
            if (resA != HALOperationResult::IfConditionFalse)
                return resA;
            // only if A was false, evaluate B and return its result
            LogicExecNode* logicExecNodeB = static_cast<LogicExecNode*>(logicExecNode->childB);
            return logicExecNodeB->handler(logicExecNodeB);
        }
        HALOperationResult LogicExecNode::EvalOr_CC(void* context) { // CC
#ifdef DEBUG_LOGIC_EXEC
            printf("\nEvalOr_CC\n");
#endif
            LogicExecNode* logicExecNode = static_cast<LogicExecNode*>(context);
            HALOperationResult resA = Eval_Calc(logicExecNode->childA);
            // short-circuit: if A is not false, return immediately the result (either True or any error)
            if (resA != HALOperationResult::IfConditionFalse)
                return resA;
            return Eval_Calc(logicExecNode->childB);
        }

        void LogicExecNode::GetSetHandlers(LogicRPNNode& lrpnNode, LogicExecHandler& handler, Deleter2x& deleter) {
            LogicRPNNode* childA = lrpnNode.childA;
            LogicRPNNode* childB = lrpnNode.childB;
            ExpressionToken* op = lrpnNode.op;

            if (childA->calcRPNStartIndex != -1 && childB->calcRPNStartIndex != -1) { // CC

                deleter = DeleteAs<CalcRPN, CalcRPN>;
                if (op->type == ExpTokenType::LogicalAnd)
                    handler = &EvalAnd_CC;
                else
                    handler = &EvalOr_CC;

            } else if (childA->calcRPNStartIndex == -1 && childB->calcRPNStartIndex == -1) { // CC

                deleter = DeleteAs<LogicExecNode, CalcRPN>;
                if (op->type == ExpTokenType::LogicalAnd)
                    handler = &EvalAnd_LC;
                else
                    handler = &EvalOr_LC;

            } else if (childA->calcRPNStartIndex != -1 && childB->calcRPNStartIndex == -1) { // CL

                deleter = DeleteAs<CalcRPN, LogicExecNode>;
                if (op->type == ExpTokenType::LogicalAnd)
                    handler = &EvalAnd_CL;
                else
                    handler = &EvalOr_CL;

            } else if (childA->calcRPNStartIndex == -1 && childB->calcRPNStartIndex != -1) { // LC

                deleter = DeleteAs<LogicExecNode, LogicExecNode>;
                if (op->type == ExpTokenType::LogicalAnd)
                    handler = &EvalAnd_LL;
                else
                    handler = &EvalOr_LL;

            }
        }

        LogicRPNNode::LogicRPNNode()
            : calcRPNStartIndex(-1),calcRPNEndIndex(-1), childA(nullptr), childB(nullptr), op(nullptr) {}

        void LogicRPNNode::Set(ExpressionTokens* tokens, int startIndex, int endIndex) {
            calcRPNStartIndex = startIndex;
            calcRPNEndIndex = endIndex;
            childA = nullptr;
            childB = nullptr;
            op = nullptr;
        }

        void LogicRPNNode::Set(ExpressionToken* opTok, LogicRPNNode* lhs, LogicRPNNode* rhs) {
            calcRPNStartIndex = -1;
            calcRPNEndIndex = -1;
            childA = lhs;
            childB = rhs;
            op = opTok;
        }

        LogicRPNNode::~LogicRPNNode() {
            // Do nothing — memory is managed by the pool
            childA = nullptr;
            childB = nullptr;
            op = nullptr;
        }
    }
}