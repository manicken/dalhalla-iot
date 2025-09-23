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
#include "../HAL_JSON_Value.h"
#include "../HAL_JSON_Device.h"
#include "../HAL_JSON_CachedDeviceAccess.h"
#include "HAL_JSON_SCRIPT_ENGINE_Support.h"
#include "HAL_JSON_SCRIPT_ENGINE_RPNStack.h"
#include "HAL_JSON_SCRIPT_ENGINE_Parser_Token.h"
#include "HAL_JSON_SCRIPT_ENGINE_Expression_Token.h"
#include "HAL_JSON_SCRIPT_ENGINE_Operators.h"

namespace HAL_JSON {
    namespace ScriptEngine {
        /** used for all value calculations */ 
        extern RPNStack<HALValue> halValueStack;
        using RPNHandler = HALOperationResult(*)(void*);
        
        
        template <typename Op>
        HALOperationResult Operation_Handler(void* context) {
            int sp = halValueStack.sp; // micro-optimization - store locally 
#ifdef HAL_JSON_SCRIPTS_STRUCTURES_RPN_STACK_SAFETY_CHECKS
            if (sp < 2) { return HALOperationResult::StackUnderflow; }    // underflow check
            if (sp >= halValueStack.size){ return HALOperationResult::StackOverflow; }   // overflow check before push
#endif
            HALValue* itemPtr = &halValueStack.items[sp];
            HALValue b = *--itemPtr;
            HALValue a = *--itemPtr;
            *itemPtr++ = Op::apply(a, b);
            halValueStack.sp = sp - 1;
            return HALOperationResult::Success;
        }

        template <typename Op>
        HALOperationResult Division_And_Modulus_Operation_Handler(void* context) {
            int sp = halValueStack.sp; // micro-optimization - store locally 
#ifdef HAL_JSON_SCRIPTS_STRUCTURES_RPN_STACK_SAFETY_CHECKS
            if (sp < 2) { return HALOperationResult::StackUnderflow; }    // underflow check
            if (sp >= halValueStack.size){ return HALOperationResult::StackOverflow; }   // overflow check before push
#endif
            HALValue* itemPtr = &halValueStack.items[sp];
            HALValue b = *--itemPtr;
            if (b.asInt() == 0) {
                // Log to GlobalLogger about divide by zero
                return HALOperationResult::DivideByZero;
            }
            HALValue a = *--itemPtr;
            *itemPtr++ = Op::apply(a, b);
            halValueStack.sp = sp - 1;
            return HALOperationResult::Success;
        }

        struct CalcRPNToken {
            HAL_JSON_NOCOPY_NOMOVE(CalcRPNToken);
            /** 
             * in this case this will either be:
             * CachedDeviceAccess
             * HALValue
             */
            void* context;
            RPNHandler handler;
            Deleter deleter;

            

            static HALOperationResult GetAndPushVariableValue_Handler(void* context);
            static HALOperationResult GetAndPushConstValue_Handler(void* context);

           /* static HALOperationResult Calc_Addition_Operation_Handler(void* context);
            static HALOperationResult Calc_Subtract_Operation_Handler(void* context);
            static HALOperationResult Calc_Multiply_Operation_Handler(void* context);
            static HALOperationResult Calc_Divide_Operation_Handler(void* context);
            static HALOperationResult Calc_Modulus_Operation_Handler(void* context);

            static HALOperationResult Calc_BitwiseAnd_Operation_Handler(void* context);
            static HALOperationResult Calc_BitwiseOr_Operation_Handler(void* context);
            static HALOperationResult Calc_BitwiseExOr_Operation_Handler(void* context);
            static HALOperationResult Calc_BitwiseLeftShift_Operation_Handler(void* context);
            static HALOperationResult Calc_BitwiseRightShift_Operation_Handler(void* context);

            static HALOperationResult Compare_NotEqualsTo_Operation_Handler(void* context);
            static HALOperationResult Compare_EqualsTo_Operation_Handler(void* context);
            static HALOperationResult Compare_LessThan_Operation_Handler(void* context);
            static HALOperationResult Compare_GreaterThan_Operation_Handler(void* context);
            static HALOperationResult Compare_LessThanOrEqual_Operation_Handler(void* context);
            static HALOperationResult Compare_GreaterThanOrEqual_Operation_Handler(void* context);*/

            static RPNHandler GetRPN_OperatorFunction(ExpTokenType type);

            CalcRPNToken();
            ~CalcRPNToken();
        };

        struct CalcRPN {
            std::string calcRPNstr;
            
            CalcRPNToken* items;
            int count;

            CalcRPN(CalcRPN&) = delete;
            CalcRPN(ExpressionTokens* tokens, int startIndex, int endIndex);
            ~CalcRPN();
            HALOperationResult DoCalc();
        };
    }
}