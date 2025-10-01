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

#include "HAL_JSON_SCRIPT_ENGINE_CalcRPNToken.h"

#define HAL_JSON_SCRIPTS_STRUCTURES_RPN_STACK_SAFETY_CHECKS

//#define DEBUG_CALCRPN_EXEC

namespace HAL_JSON {
    namespace ScriptEngine {
        RPNStack<HALValue> halValueStack;

        CalcRPNToken::CalcRPNToken() { }
        CalcRPNToken::~CalcRPNToken()
        {
            
            if (deleter) deleter(context);
        }
        CalcRPN::CalcRPN(ExpressionTokens* tokens, int startIndex, int endIndex) {
            calcRPNstr = PrintExpressionTokensOneRow(*tokens, startIndex, endIndex);
            count = endIndex - startIndex;
            int tokensCurrCount = tokens->currentCount;
            //if (count != tokensCurrCount) // this is not really a error here
            //    printf("\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ERROR count != tokensCurrCount ****************\n");
#ifdef DEBUG_CALCRPN_EXEC
            printf("CalcRPN::CalcRPN constr - count:%d, tokensCurrCount:%d, rpn:%s\n", count, tokensCurrCount, PrintExpressionTokensOneRow(*tokens,startIndex, endIndex).c_str());
#endif
            items = new CalcRPNToken[count];
            int calcRPNindex = 0;
            ExpressionToken* tokenItems = tokens->items;
            for (int i=startIndex;i<endIndex;i++, calcRPNindex++) {
                ExpressionToken& expToken = tokenItems[i];
                CalcRPNToken& calcRPNtoken = items[calcRPNindex];
                if (expToken.type == ExpTokenType::VarOperand)
                {
                    ZeroCopyString varOperand;
                    ZeroCopyString funcName = expToken;
                    varOperand = funcName.SplitOffHead('#');
                    CachedDeviceAccess* cda = new CachedDeviceAccess(varOperand, funcName);

                    calcRPNtoken.context = cda;
                    calcRPNtoken.handler = &CalcRPNToken::GetAndPushVariableValue_Handler;
                    calcRPNtoken.deleter = DeleteAs<CachedDeviceAccess>;
                }
                else if (expToken.type == ExpTokenType::ConstValOperand)
                {
                    NumberResult constNumber = expToken.ConvertStringToNumber();
                    HALValue* value = new HALValue();
                    if (constNumber.type == NumberType::FLOAT)
                        value->set(constNumber.f32);
                    else if (constNumber.type == NumberType::INT32)
                        value->set(constNumber.i32);
                    else if (constNumber.type == NumberType::UINT32)
                        value->set(constNumber.u32);
                    else { // should never happend
                        std::string msg = expToken.ToString();
                        GlobalLogger.Error(F("fail of converting constant default is set to one"), msg.c_str()); // remainder the string is copied internally here
                        value->set(1); // default one so any divide by zero would not happend
                    }

                    calcRPNtoken.context = value;
                    calcRPNtoken.handler = &CalcRPNToken::GetAndPushConstValue_Handler;
                    calcRPNtoken.deleter = DeleteAs<HALValue>;
                }
                else
                {
                    calcRPNtoken.context = nullptr; // not used here
                    calcRPNtoken.handler = CalcRPNToken::GetRPN_OperatorFunction(expToken.type);
                    calcRPNtoken.deleter = nullptr; // not used here
                }
            }
        }
        CalcRPN::~CalcRPN() {
            //printf("\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!CalcRPN destructor was run\n");
            delete[] items;
        }
        HALOperationResult CalcRPN::DoCalc() {
#ifdef DEBUG_CALCRPN_EXEC
            printf("CalcRPN::DoCalc - rpn is:%s", calcRPNstr.c_str());
#endif
            int calcRPNcount = count; // deref here for faster access
            CalcRPNToken* calcItems = items; // deref here for faster access
            halValueStack.sp = 0; // 'clear' stack before use
            for (int i=0;i<calcRPNcount;i++) {
                if (calcItems[i].handler == nullptr) {
                    printf("CalcRPN::DoCalc - handler was nullptr at index:%d of total:%d, rpn is:%s\n", i, calcRPNcount, calcRPNstr.c_str());
                    return HALOperationResult::HandlerWasNullPtr;
                }
                HALOperationResult res = calcItems[i].handler(calcItems[i].context);
                if (res != HALOperationResult::Success)
                    return res;
            }
#ifdef DEBUG_CALCRPN_EXEC
            HALValue val;
            halValueStack.GetFinalResult(val);
            if (val.getType() == HALValue::Type::FLOAT)
                printf(" result: float(%.6f)\n", val.asFloat());
            else if (val.getType() == HALValue::Type::INT)
                printf(" result: int(%d)\n", val.asInt());
            else if (val.getType() == HALValue::Type::UINT)
                printf(" result: uint:(%u)\n", val.asUInt());
#endif
            return HALOperationResult::Success;
        }

        HALOperationResult CalcRPNToken::GetAndPushVariableValue_Handler(void* context) {
            CachedDeviceAccess* item = static_cast<CachedDeviceAccess*>(context);
            HALValue value;
            if (item->valueDirectAccessPtr != nullptr) {
                value = *(item->valueDirectAccessPtr);
            }
            else if (item->readToHalValueFunc != nullptr) {
                Device* device = item->GetDevice();
                HALOperationResult result = item->readToHalValueFunc(device, value);
                if (result != HALOperationResult::Success) return result;
            } else {
                Device* device = item->GetDevice();
                HALOperationResult result = device->read(value);
                if (result != HALOperationResult::Success) return result;
            }
                
#ifdef HAL_JSON_SCRIPTS_STRUCTURES_RPN_STACK_SAFETY_CHECKS
            if (halValueStack.sp >= halValueStack.size){ return HALOperationResult::StackOverflow; }   // overflow check before push
#endif
            halValueStack.items[halValueStack.sp++] = value;
            return HALOperationResult::Success;
        }

        HALOperationResult CalcRPNToken::GetAndPushConstValue_Handler(void* context) {
            HALValue* item = static_cast<HALValue*>(context);
#ifdef HAL_JSON_SCRIPTS_STRUCTURES_RPN_STACK_SAFETY_CHECKS
            if (halValueStack.sp >= halValueStack.size){ return HALOperationResult::StackOverflow; }   // overflow check before push
#endif
            //printf("\nsetting constant value:%.6f\n", item->asFloat());
            halValueStack.items[halValueStack.sp++] = *item;
            return HALOperationResult::Success;
        }



/*
        HALOperationResult CalcRPNToken::Calc_Addition_Operation_Handler(void* context) {
            int sp = halValueStack.sp; // micro-optimization - store locally 
#ifdef HAL_JSON_SCRIPTS_STRUCTURES_RPN_STACK_SAFETY_CHECKS
            if (sp < 2) { return HALOperationResult::StackUnderflow; }    // underflow check
            if (sp >= halValueStack.size){ return HALOperationResult::StackOverflow; }   // overflow check before push
#endif
            HALValue* itemPtr = &halValueStack.items[sp];
            HALValue b = *--itemPtr;
            HALValue a = *--itemPtr;
            *itemPtr++ = (a + b);
            halValueStack.sp = sp - 1;
            return HALOperationResult::Success;
        }
        HALOperationResult CalcRPNToken::Calc_Subtract_Operation_Handler(void* context) {
            int sp = halValueStack.sp; // micro-optimization - store locally 
#ifdef HAL_JSON_SCRIPTS_STRUCTURES_RPN_STACK_SAFETY_CHECKS
            if (sp < 2) { return HALOperationResult::StackUnderflow; }    // underflow check
            if (sp >= halValueStack.size){ return HALOperationResult::StackOverflow; }   // overflow check before push
#endif
            HALValue* itemPtr = &halValueStack.items[sp];
            HALValue b = *--itemPtr;
            HALValue a = *--itemPtr;
            *itemPtr++ = (a - b);
            halValueStack.sp = sp - 1;
            return HALOperationResult::Success;
        }
        HALOperationResult CalcRPNToken::Calc_Multiply_Operation_Handler(void* context) {
            int sp = halValueStack.sp; // micro-optimization - store locally 
#ifdef HAL_JSON_SCRIPTS_STRUCTURES_RPN_STACK_SAFETY_CHECKS
            if (sp < 2) { return HALOperationResult::StackUnderflow; }    // underflow check
            if (sp >= halValueStack.size){ return HALOperationResult::StackOverflow; }   // overflow check before push
#endif
            HALValue* itemPtr = &halValueStack.items[sp];
            HALValue b = *--itemPtr;
            HALValue a = *--itemPtr;
            *itemPtr++ = (a * b);
            halValueStack.sp = sp - 1;
            return HALOperationResult::Success;
        }
        HALOperationResult CalcRPNToken::Calc_Divide_Operation_Handler(void* context) {
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
            *itemPtr++ = (a / b);
            halValueStack.sp = sp - 1;
            return HALOperationResult::Success;
        }
        HALOperationResult CalcRPNToken::Calc_Modulus_Operation_Handler(void* context) {
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
            *itemPtr++ = (a % b);
            halValueStack.sp = sp - 1;
            return HALOperationResult::Success;
        }
        HALOperationResult CalcRPNToken::Calc_BitwiseAnd_Operation_Handler(void* context) {
            int sp = halValueStack.sp; // micro-optimization - store locally 
#ifdef HAL_JSON_SCRIPTS_STRUCTURES_RPN_STACK_SAFETY_CHECKS
            if (sp < 2) { return HALOperationResult::StackUnderflow; }    // underflow check
            if (sp >= halValueStack.size){ return HALOperationResult::StackOverflow; }   // overflow check before push
#endif
            HALValue* itemPtr = &halValueStack.items[sp];
            HALValue b = *--itemPtr;
            HALValue a = *--itemPtr;
            *itemPtr++ = (a & b);
            halValueStack.sp = sp - 1;
            return HALOperationResult::Success;
        }
        HALOperationResult CalcRPNToken::Calc_BitwiseOr_Operation_Handler(void* context) {
            int sp = halValueStack.sp; // micro-optimization - store locally 
#ifdef HAL_JSON_SCRIPTS_STRUCTURES_RPN_STACK_SAFETY_CHECKS
            if (sp < 2) { return HALOperationResult::StackUnderflow; }    // underflow check
            if (sp >= halValueStack.size){ return HALOperationResult::StackOverflow; }   // overflow check before push
#endif
            HALValue* itemPtr = &halValueStack.items[sp];
            HALValue b = *--itemPtr;
            HALValue a = *--itemPtr;
            *itemPtr++ = a | b;
            halValueStack.sp = sp - 1;
            return HALOperationResult::Success;
        }
        HALOperationResult CalcRPNToken::Calc_BitwiseExOr_Operation_Handler(void* context) {
            int sp = halValueStack.sp; // micro-optimization - store locally 
#ifdef HAL_JSON_SCRIPTS_STRUCTURES_RPN_STACK_SAFETY_CHECKS
            if (sp < 2) { return HALOperationResult::StackUnderflow; }    // underflow check
            if (sp >= halValueStack.size){ return HALOperationResult::StackOverflow; }   // overflow check before push
#endif
            HALValue* itemPtr = &halValueStack.items[sp];
            HALValue b = *--itemPtr;
            HALValue a = *--itemPtr;
            *itemPtr++ = (a ^ b);
            halValueStack.sp = sp - 1;
            return HALOperationResult::Success;
        }
        HALOperationResult CalcRPNToken::Calc_BitwiseLeftShift_Operation_Handler(void* context) {
            int sp = halValueStack.sp; // micro-optimization - store locally 
#ifdef HAL_JSON_SCRIPTS_STRUCTURES_RPN_STACK_SAFETY_CHECKS
            if (sp < 2) { return HALOperationResult::StackUnderflow; }    // underflow check
            if (sp >= halValueStack.size){ return HALOperationResult::StackOverflow; }   // overflow check before push
#endif
            HALValue* itemPtr = &halValueStack.items[sp];
            HALValue b = *--itemPtr;
            HALValue a = *--itemPtr;
            *itemPtr++ = (a << b);
            halValueStack.sp = sp - 1;
            return HALOperationResult::Success;
        }
        HALOperationResult CalcRPNToken::Calc_BitwiseRightShift_Operation_Handler(void* context) {
            int sp = halValueStack.sp; // micro-optimization - store locally 
#ifdef HAL_JSON_SCRIPTS_STRUCTURES_RPN_STACK_SAFETY_CHECKS
            if (sp < 2) { return HALOperationResult::StackUnderflow; }    // underflow check
            if (sp >= halValueStack.size){ return HALOperationResult::StackOverflow; }   // overflow check before push
#endif
            HALValue* itemPtr = &halValueStack.items[sp];
            HALValue b = *--itemPtr;
            HALValue a = *--itemPtr;
            *itemPtr++ = (a >> b);
            halValueStack.sp = sp - 1;
            return HALOperationResult::Success;
        }

        HALOperationResult CalcRPNToken::Compare_NotEqualsTo_Operation_Handler(void* context) {
            int sp = halValueStack.sp; // micro-optimization - store locally 
#ifdef HAL_JSON_SCRIPTS_STRUCTURES_RPN_STACK_SAFETY_CHECKS
            if (sp < 2) { return HALOperationResult::StackUnderflow; }    // underflow check
            if (sp >= halValueStack.size){ return HALOperationResult::StackOverflow; }   // overflow check before push
#endif
            HALValue* itemPtr = &halValueStack.items[sp];
            HALValue b = *--itemPtr;
            HALValue a = *--itemPtr;
            *itemPtr++ = (a != b);
            halValueStack.sp = sp - 1;
            return HALOperationResult::Success;
        }
        HALOperationResult CalcRPNToken::Compare_EqualsTo_Operation_Handler(void* context) {
            int sp = halValueStack.sp; // micro-optimization - store locally 
#ifdef HAL_JSON_SCRIPTS_STRUCTURES_RPN_STACK_SAFETY_CHECKS
            if (sp < 2) { return HALOperationResult::StackUnderflow; }    // underflow check
            if (sp >= halValueStack.size){ return HALOperationResult::StackOverflow; }   // overflow check before push
#endif
            HALValue* itemPtr = &halValueStack.items[sp];
            HALValue b = *--itemPtr;
            HALValue a = *--itemPtr;
            *itemPtr++ = (a == b);
            halValueStack.sp = sp - 1;
            return HALOperationResult::Success;
        }
        HALOperationResult CalcRPNToken::Compare_LessThan_Operation_Handler(void* context) {
            int sp = halValueStack.sp; // micro-optimization - store locally 
#ifdef HAL_JSON_SCRIPTS_STRUCTURES_RPN_STACK_SAFETY_CHECKS
            if (sp < 2) { return HALOperationResult::StackUnderflow; }    // underflow check
            if (sp >= halValueStack.size){ return HALOperationResult::StackOverflow; }   // overflow check before push
#endif
            HALValue* itemPtr = &halValueStack.items[sp];
            HALValue b = *--itemPtr;
            HALValue a = *--itemPtr;
            *itemPtr++ = (a < b);
            halValueStack.sp = sp - 1;
            return HALOperationResult::Success;
        }
        HALOperationResult CalcRPNToken::Compare_GreaterThan_Operation_Handler(void* context) {
            int sp = halValueStack.sp; // micro-optimization - store locally 
#ifdef HAL_JSON_SCRIPTS_STRUCTURES_RPN_STACK_SAFETY_CHECKS
            if (sp < 2) { return HALOperationResult::StackUnderflow; }    // underflow check
            if (sp >= halValueStack.size){ return HALOperationResult::StackOverflow; }   // overflow check before push
#endif
            HALValue* itemPtr = &halValueStack.items[sp];
            HALValue b = *--itemPtr;
            HALValue a = *--itemPtr;
            *itemPtr++ = (a > b);
            halValueStack.sp = sp - 1;
            return HALOperationResult::Success;
        }
        HALOperationResult CalcRPNToken::Compare_LessThanOrEqual_Operation_Handler(void* context) {
            int sp = halValueStack.sp; // micro-optimization - store locally 
#ifdef HAL_JSON_SCRIPTS_STRUCTURES_RPN_STACK_SAFETY_CHECKS
            if (sp < 2) { return HALOperationResult::StackUnderflow; }    // underflow check
            if (sp >= halValueStack.size){ return HALOperationResult::StackOverflow; }   // overflow check before push
#endif
            HALValue* itemPtr = &halValueStack.items[sp];
            HALValue b = *--itemPtr;
            HALValue a = *--itemPtr;
            *itemPtr++ = (a <= b);
            halValueStack.sp = sp - 1;
            return HALOperationResult::Success;
        }
        HALOperationResult CalcRPNToken::Compare_GreaterThanOrEqual_Operation_Handler(void* context) {
            int sp = halValueStack.sp; // micro-optimization - store locally 
#ifdef HAL_JSON_SCRIPTS_STRUCTURES_RPN_STACK_SAFETY_CHECKS
            if (sp < 2) { return HALOperationResult::StackUnderflow; }    // underflow check
            if (sp >= halValueStack.size){ return HALOperationResult::StackOverflow; }   // overflow check before push
#endif
            HALValue* itemPtr = &halValueStack.items[sp];
            HALValue b = *--itemPtr;
            HALValue a = *--itemPtr;
            *itemPtr++ = (a >= b);
            halValueStack.sp = sp - 1;
            return HALOperationResult::Success;
        }

        RPNHandler CalcRPNToken::GetRPN_OperatorFunction(ExpTokenType type) {
            switch (type) {
                case ExpTokenType::CompareEqualsTo: return &Compare_EqualsTo_Operation_Handler;
                case ExpTokenType::CompareNotEqualsTo: return &Compare_NotEqualsTo_Operation_Handler;
                case ExpTokenType::CompareLessThan: return &Compare_LessThan_Operation_Handler;
                case ExpTokenType::CompareGreaterThan: return &Compare_GreaterThan_Operation_Handler;
                case ExpTokenType::CompareLessThanOrEqual: return &Compare_LessThanOrEqual_Operation_Handler;
                case ExpTokenType::CompareGreaterThanOrEqual: return &Compare_GreaterThanOrEqual_Operation_Handler;
                case ExpTokenType::CalcPlus: return &Calc_Addition_Operation_Handler;
                case ExpTokenType::CalcMinus: return &Calc_Subtract_Operation_Handler;
                case ExpTokenType::CalcMultiply: return &Calc_Multiply_Operation_Handler;
                case ExpTokenType::CalcDivide: return &Calc_Divide_Operation_Handler;
                case ExpTokenType::CalcModulus: return &Calc_Modulus_Operation_Handler;
                case ExpTokenType::CalcBitwiseAnd: return &Calc_BitwiseAnd_Operation_Handler;
                case ExpTokenType::CalcBitwiseOr: return &Calc_BitwiseOr_Operation_Handler;
                case ExpTokenType::CalcBitwiseExOr: return &Calc_BitwiseExOr_Operation_Handler;
                case ExpTokenType::CalcBitwiseLeftShift: return &Calc_BitwiseLeftShift_Operation_Handler;
                case ExpTokenType::CalcBitwiseRightShift: return &Calc_BitwiseRightShift_Operation_Handler;
                default: return nullptr; // handled by caller
            }
        }*/

        RPNHandler CalcRPNToken::GetRPN_OperatorFunction(ExpTokenType type) {
            switch (type) {
                case ExpTokenType::CompareEqualsTo: return &Operation_Handler<OpCompEqual>;
                case ExpTokenType::CompareNotEqualsTo: return &Operation_Handler<OpCompNotEqual>;
                case ExpTokenType::CompareLessThan: return &Operation_Handler<OpCompLessThan>;
                case ExpTokenType::CompareGreaterThan: return &Operation_Handler<OpCompGreaterThan>;
                case ExpTokenType::CompareLessThanOrEqual: return &Operation_Handler<OpCompLessOrEqual>;
                case ExpTokenType::CompareGreaterThanOrEqual: return &Operation_Handler<OpCompGreaterOrEqual>;
                case ExpTokenType::CalcPlus: return &Operation_Handler<OpAdd>;
                case ExpTokenType::CalcMinus: return &Operation_Handler<OpSub>;
                case ExpTokenType::CalcMultiply: return &Operation_Handler<OpMul>;
                case ExpTokenType::CalcDivide: return &Division_And_Modulus_Operation_Handler<OpDiv>;
                case ExpTokenType::CalcModulus: return &Division_And_Modulus_Operation_Handler<OpMod>;
                case ExpTokenType::CalcBitwiseAnd: return &Operation_Handler<OpBitAnd>;
                case ExpTokenType::CalcBitwiseOr: return &Operation_Handler<OpBitOr>;
                case ExpTokenType::CalcBitwiseExOr: return &Operation_Handler<OpBitExOr>;
                case ExpTokenType::CalcBitwiseLeftShift: return &Operation_Handler<OpBitLshift>;
                case ExpTokenType::CalcBitwiseRightShift: return &Operation_Handler<OpBitRshift>;
                default: return nullptr; // handled by caller
            }
        }

    }
}