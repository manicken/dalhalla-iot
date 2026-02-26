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

#pragma once

#include "../../Core/HAL_JSON_Device.h" // HALOperationResult
#include "../../Support/HAL_JSON_DeleterTemplate.h" // Deleter
#include "../Parser/HAL_JSON_SCRIPT_ENGINE_Expression_Token.h" // ExpressionTokens, ExpressionToken

namespace HAL_JSON {
    namespace ScriptEngine {
        using LogicExecHandler = HALOperationResult(*)(void*);

        /** only both for development tests and as a temp structure*/
        struct LogicRPNNode {

            //ExpressionTokens* calcRPN;  // leaf if this i non nullptr
            int calcRPNStartIndex; // leaf if this is not -1
            int calcRPNEndIndex;
            /** is owned by Expressions "static" class */
            LogicRPNNode* childA;   // nested nodes, nullptr when leaf
            /** is owned by Expressions "static" class */
            LogicRPNNode* childB;   // nested nodes, nullptr when leaf
            /** is owned by Expressions "static" class */
            ExpressionToken* op;                     // "&&" or "||", nullptr when leaf

            LogicRPNNode();
            void Set(ExpressionTokens* tokens, int startIndex, int endIndex);
            void Set(ExpressionToken* opTok, LogicRPNNode* lhs, LogicRPNNode* rhs);
            ~LogicRPNNode();
        };

        struct LogicExecNode {
            void* childA;   // either LogicExecNode* or CalcRPN Context*
            void* childB;   // either LogicExecNode* or CalcRPN Context*
            LogicExecHandler handler;  // function pointer to EvalAnd/Or variant
            Deleter2x deleter; // handler that delete the childs depending on set types

            LogicExecNode(LogicExecNode&) = delete;
            
            LogicExecNode(ExpressionTokens* expTokens, LogicRPNNode* logicRPNnode);

            static HALOperationResult Eval_Calc(void* context);
            static HALOperationResult EvalAnd_LL(void* context);  // LL (logic logic)
            static HALOperationResult EvalAnd_LC(void* context); // LC (logic calc)
            static HALOperationResult EvalAnd_CL(void* context); // CL (calc logic)
            static HALOperationResult EvalAnd_CC(void* context); // CC (calc calc)

            static HALOperationResult EvalOr_LL(void* context);    // LL
            static HALOperationResult EvalOr_LC(void* context);  // LC
            static HALOperationResult EvalOr_CL(void* context);  // CL
            static HALOperationResult EvalOr_CC(void* context); // CC

            static void GetSetHandlers(LogicRPNNode& lrpnNode, LogicExecHandler& handler, Deleter2x& deleter);
        };

        

        
    }
}