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

#include "DALHAL_SCRIPT_ENGINE_CalcRPNToken.h"
#include "DALHAL_SCRIPT_ENGINE_RPNStack.h" //contains the instance of halValueStack
#include "../../Core/DALHAL_CachedDeviceRead.h"
#include "../../Core/DALHAL_Device.h"
#include "../../Core/DALHAL_Manager.h"
#include "../../Support/DALHAL_Logger.h"

#define DALHAL_SCRIPTS_STRUCTURES_RPN_STACK_SAFETY_CHECKS

//#define DEBUG_CALCRPN_EXEC

namespace DALHAL {
    namespace ScriptEngine {

        void CalcRPNToken::Set(ExpressionToken& expToken) {
            if (expToken.type == ExpTokenType::VarOperand) {
                SetAsCachedDeviceAccess(expToken);
            } else if (expToken.type == ExpTokenType::ConstValOperand) {
                SetAsConstValue(expToken);
            } else {
                handler = CalcRPNToken::GetRPN_OperatorFunction(expToken.type);
                deleter = nullptr; // not used here
            }
        }

        void CalcRPNToken::SetAsCachedDeviceAccess(ExpressionToken& expToken)
        {
            if (expToken.ContainsChar('[')) {
                CachedDeviceRead* cdr = new CachedDeviceRead();
                cdr->Set(expToken); // dont need to check if return true as at this stage it's valid

                // Assign context, handler, and deleter
                context = cdr;
                handler = &CalcRPNToken::GetAndPushVariableValue_Handler;
                deleter = DeleteAs<CachedDeviceRead>;
                
            } else if (expToken.ContainsChar('#')) {
                ZeroCopyString funcName = expToken;
                ZeroCopyString varOperand = funcName.SplitOffHead('#');
                UIDPath uidPath(varOperand);
                Device* device = nullptr;
                DeviceFindResult devFindRes = Manager::findDevice(uidPath, device);

                if (devFindRes != DeviceFindResult::Success) { // failsafe
                    printf("@CalcRPNToken - CachedDeviceAccess - %s:>>%s<<\n", DeviceFindResultToString(devFindRes), uidPath.ToString().c_str());
                    handler = &DummyHandler;
                    return;
                }
                // only a funcname call
                ReadToHALValue_Function_Context* ctx = new ReadToHALValue_Function_Context();
                ctx->device = device;
                ctx->handler = device->GetReadToHALValue_Function(funcName);
                // Assign context, handler, and deleter
                context = ctx;
                deleter = DeleteAs<ReadToHALValue_Function_Context>;
                handler = &CalcRPNToken::GetAndPushReadToHALValue_Function_Context_Handler;
            } else {
                deleter = nullptr; // the context is allways non owning
                //printf("\nCalcRPNToken - non bracket non funcname accessor\n");
                UIDPath uidPath(expToken);
                Device* device = nullptr;
                DeviceFindResult devFindRes = Manager::findDevice(uidPath, device);

                if (devFindRes != DeviceFindResult::Success) {  // failsafe
                    printf("\nCalcRPNToken - @non bracket non funcname accessor - %d:>>%s<<\n", DeviceFindResultToString(devFindRes), uidPath.ToString().c_str());
                    handler = &DummyHandler;
                    return;
                }
                HALValue* directPtr = device->GetValueDirectAccessPtr();
                if (directPtr != nullptr) {
                    //printf("\nCalcRPNToken - Did use direct access ptr%s\n", expToken.ToString().c_str());
                    context = directPtr;
                    handler = &CalcRPNToken::GetAndPushValuePtr_Handler;
                } else {
                    context = device;
                    handler = &CalcRPNToken::GetAndPushDeviceReadVariableValue_Handler;
                }
            }
        }

        void CalcRPNToken::SetAsConstValue(ExpressionToken& expToken)
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
                value->set((uint32_t)1); // default one so any divide by zero would not happend
            }

            context = value;
            handler = &CalcRPNToken::GetAndPushValuePtr_Handler;
            deleter = DeleteAs<HALValue>;
        }

        HALOperationResult CalcRPNToken::DummyHandler(void* context) {
            
            return HALOperationResult::HandlerWasDummy;
        }

        CalcRPNToken::CalcRPNToken(): context(nullptr), handler(&DummyHandler), deleter(nullptr) { }
        CalcRPNToken::~CalcRPNToken()
        {
            if (deleter) deleter(context);
            // avoid dangling pointers
            deleter = nullptr;
            context = nullptr;
            handler = nullptr;
        }

        HALOperationResult CalcRPNToken::GetAndPushReadToHALValue_Function_Context_Handler(void* context) {
            ReadToHALValue_Function_Context* cdaItem = static_cast<ReadToHALValue_Function_Context*>(context);
            HALValue value;
            HALOperationResult result = cdaItem->handler(cdaItem->device, value);
            if (result != HALOperationResult::Success) return result;
                
#ifdef DALHAL_SCRIPTS_STRUCTURES_RPN_STACK_SAFETY_CHECKS
            if (halValueStack.sp >= halValueStack.size){ return HALOperationResult::StackOverflow; }   // overflow check before push
#endif
            halValueStack.items[halValueStack.sp++] = value;
            return HALOperationResult::Success;
        }

        HALOperationResult CalcRPNToken::GetAndPushDeviceReadVariableValue_Handler(void* context) {
            Device* cdaItem = static_cast<Device*>(context);
            HALValue value;
            HALOperationResult result = cdaItem->read(value);
            if (result != HALOperationResult::Success) return result;
                
#ifdef DALHAL_SCRIPTS_STRUCTURES_RPN_STACK_SAFETY_CHECKS
            if (halValueStack.sp >= halValueStack.size){ return HALOperationResult::StackOverflow; }   // overflow check before push
#endif
            halValueStack.items[halValueStack.sp++] = value;
            return HALOperationResult::Success;
        }

        HALOperationResult CalcRPNToken::GetAndPushVariableValue_Handler(void* context) {
            CachedDeviceRead* cdrItem = static_cast<CachedDeviceRead*>(context);
            HALValue value;
            HALOperationResult result = cdrItem->ReadSimple(value);
            if (result != HALOperationResult::Success) return result;
                
#ifdef DALHAL_SCRIPTS_STRUCTURES_RPN_STACK_SAFETY_CHECKS
            if (halValueStack.sp >= halValueStack.size){ return HALOperationResult::StackOverflow; }   // overflow check before push
#endif
            halValueStack.items[halValueStack.sp++] = value;
            return HALOperationResult::Success;
        }

        HALOperationResult CalcRPNToken::GetAndPushValuePtr_Handler(void* context) {
#ifdef DALHAL_SCRIPTS_STRUCTURES_RPN_STACK_SAFETY_CHECKS
            if (halValueStack.sp >= halValueStack.size){ return HALOperationResult::StackOverflow; }   // overflow check before push
#endif
            //printf("\nsetting constant value:%.6f\n", item->asFloat());
            halValueStack.items[halValueStack.sp++] = *(static_cast<HALValue*>(context));
            return HALOperationResult::Success;
        }

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
                default: return &DummyHandler;
            }
        }

    }
}