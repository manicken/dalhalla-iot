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
#include <Arduino.h>
#include "../../HAL_JSON_Device.h" // HALOperationResult
#include "../HAL_JSON_SCRIPT_ENGINE_Support.h"
#include "HAL_JSON_SCRIPT_ENGINE_LogicExecNode.h"
#include "../Parser/HAL_JSON_SCRIPT_ENGINE_Expression_Token.h"
#include "../Parser/HAL_JSON_SCRIPT_ENGINE_Parser_Expressions.h"

#include "../Parser/HAL_JSON_SCRIPT_ENGINE_Script_Token.h"

namespace HAL_JSON {
    namespace ScriptEngine {

        // Forward declare StatementBlock
        struct StatementBlock;

        /** collection of StatementBlock(s) */
        struct BranchBlock
        {
            HAL_JSON_NOCOPY_NOMOVE(BranchBlock);

            StatementBlock* items;
            int itemsCount;

            /** used to execute all StatementBlock Items one after annother */
            HALOperationResult Exec(void);

            BranchBlock();
            ~BranchBlock();
        };

        struct ConditionalBranch : public BranchBlock
        {
            HAL_JSON_NOCOPY_NOMOVE(ConditionalBranch);

            /** is either LogicExecNode or CalcCompareRPN (pure without logic) */
            void* context;
            /** used to delete the context depending on type */
            Deleter deleter;
            HALOperationResult (*handler)(void* context);

            void Set(ScriptTokens& tokens);

            ConditionalBranch();
            ~ConditionalBranch();
        };

        /** 
         * this is a kind of of ConditionalBranch where there are not any LogicalExpressionRPNToken list
         * there is only one instance of it inside a IfBlock item
         */
        struct UnconditionalBranch : public BranchBlock
        {
            HAL_JSON_NOCOPY_NOMOVE(UnconditionalBranch);

            UnconditionalBranch(ScriptTokens& tokens);
            ~UnconditionalBranch();
        };

        /** 
         * contains collections of ConditionalBranch and one optional UnconditionalBranch at the end
         * for example:
         * if <condition> then (ConditionalBranch)
         * 
         * elseif <condition> then (ConditionalBranch)
         * 
         * else (UnconditionalBranch)
         * 
         * endif
         * 
         * else here just contain a collection of OpBlock items
         */
        struct IfStatement
        {
            HAL_JSON_NOCOPY_NOMOVE(IfStatement);

            ConditionalBranch* branchItems;
            int branchItemsCount;
            /** set when there are a else branch defined, else it's set to nullptr */
            UnconditionalBranch* elseBranch; 
            bool elseBranchFound;

            IfStatement(ScriptTokens& tokens);
            ~IfStatement();

            static HALOperationResult Handler(void* context);
        };

    }
}