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

#include "DALHAL_SCRIPT_ENGINE_Parser_Triggers.h"

#include <DALHAL/Core/Manager/DALHAL_DeviceManager.h>

namespace DALHAL {
    namespace ScriptEngine {
        namespace Parser {

            bool Trigger::Validate(ScriptTokens& _tokens) {
                ScriptToken* tokens = _tokens.items;
                int tokenCount = _tokens.count;
                bool anyError = false;

                for (int i = 0; i < tokenCount; ++i) {
                    ScriptToken& token = tokens[i];
                    if (token.type != ScriptTokenType::On ){ continue; }
                    if (i+2 >= tokenCount) { // failsafe check
                        anyError = true;
                        token.ReportTokenError(F("Trigger-token out of bounds - error"));
                        break;
                    }
                    if (tokens[i+2].type != ScriptTokenType::Then) {
                        anyError = true;
                        tokens[i+2].ReportTokenError(F("Trigger-token expression cannot contain spaces"));
                        i+=2;
                        continue;
                    }
                    // this mean that there is only one token after the On token
                    i++;
                    ScriptToken& onExprToken = tokens[i];
                    onExprToken.type = ScriptTokenType::OnExpression; // mark it with type so that we know it's parsed

                    // first check special case
                    if (onExprToken.EqualsIC(F(DALHAL_SCRIPT_ENGINE_TRIGGER_ALLWAYS_RUN_KEYWORD))) { // usually "eachloop"
                        continue;
                    }

                    HALOperationResult res = Validate(onExprToken);
                    if (res != HALOperationResult::Success) {
                        anyError = true;
                    }
                    /*const char* triggerSeparator = onExprToken.FindChar(DALHAL_SCRIPT_ENGINE_TRIGGER_SEPARATOR);

                    if (triggerSeparator == nullptr) { // usually @
                        anyError = true;
                        onExprToken.ReportTokenError(F("Trigger-token expression missing >>>" DALHAL_SCRIPT_ENGINE_TRIGGER_SEPARATOR_STR "<<< separator"));
                        continue;
                    }
                    ZeroCopyString zcOnExpr = onExprToken; // copy "view" first
                    ZeroCopyString zcUidPath = zcOnExpr.SplitOffHead(triggerSeparator);
                    

                    HALOperationResult res = DeviceManager::GetDeviceEvent(zcUidPath, zcOnExpr, nullptr); // nullptr -> test only

                    if (res != HALOperationResult::Success) {

                        onExprToken.ReportTokenError(F("Trigger-token expression error because: "), HALOperationResultToString(res));
                        anyError = true;
                        continue;
                    }*/
                    
                }
                return anyError == false;
            }

            HALOperationResult Trigger::Validate(ScriptToken& token) {
                const char* triggerSeparator = token.FindChar(DALHAL_SCRIPT_ENGINE_TRIGGER_SEPARATOR);

                if (triggerSeparator == nullptr) { // usually @
                    token.ReportTokenError(F("Trigger-token expression missing >>>" DALHAL_SCRIPT_ENGINE_TRIGGER_SEPARATOR_STR "<<< separator"));
                    return HALOperationResult::TriggerSeparatorMissing;
                }
                ScriptToken zcOnExpr = token; // copy "view" first
                ScriptToken zcUidPath = zcOnExpr.SplitOffHead(triggerSeparator);
                
                HALOperationResult res = DeviceManager::GetDeviceEvent(zcUidPath, zcOnExpr, nullptr); // nullptr -> test only

                if (res != HALOperationResult::Success) {

                    if (res == HALOperationResult::DeviceNotFound) {
                        zcUidPath.ReportTokenError(F("Trigger-token expression cannot find device: "), zcUidPath.ToString().c_str());
                    } else if (res == HALOperationResult::ReactiveEventByNameNotFound) {
                        zcOnExpr.ReportTokenError(F("Trigger-token expression cannot find device event: "), zcOnExpr.ToString().c_str());
                    } else {
                        token.ReportTokenError(F("Trigger-token expression other error: "), HALOperationResultToString(res));
                    }
                }
                return res;
            }

        }
    }
}