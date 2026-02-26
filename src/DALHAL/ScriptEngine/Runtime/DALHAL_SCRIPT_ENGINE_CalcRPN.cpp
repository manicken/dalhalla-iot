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

#include "DALHAL_SCRIPT_ENGINE_CalcRPN.h"

namespace DALHAL {
    namespace ScriptEngine {
        

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
                items[calcRPNindex].Set(tokenItems[i]);
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
                CalcRPNToken& rpnToken = calcItems[i];
                HALOperationResult res = rpnToken.handler(rpnToken.context);
                if (res != HALOperationResult::Success) {
                    if (res == HALOperationResult::HandlerWasDummy) {
                        printf("\nCalcRPN::DoCalc - handler was Dummy at index:%d of total:%d, rpn is:%s\n", i, calcRPNcount, calcRPNstr.c_str());
                    }
                    return res;
                }
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
    }
}