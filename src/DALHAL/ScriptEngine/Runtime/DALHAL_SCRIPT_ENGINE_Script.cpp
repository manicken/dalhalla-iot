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

#include "DALHAL_SCRIPT_ENGINE_Script.h"
#include <DALHAL/Support/DALHAL_Logger.h>
#include <DALHAL/Core/Manager/DALHAL_DeviceManager.h>

#define DALHAL_SCRIPTS_STRUCTURES_RPN_STACK_SAFETY_CHECKS

namespace DALHAL {
    namespace ScriptEngine {
       
        TriggerBlock::TriggerBlock() : event(nullptr), items(nullptr), itemsCount(0) { }
        TriggerBlock::~TriggerBlock() {
            delete event;
            delete[] items;
            items = nullptr;
            event = nullptr;
            itemsCount = 0;
        }
        bool TriggerBlock::AllwaysRun(void* context) {
            //GlobalLogger.Info(F("AllwaysRun"));
            return true;
        }
        bool TriggerBlock::NeverRun(void* context) {
            //GlobalLogger.Info(F("NeverRun"));
            return false;
        }

        void TriggerBlock::Set(int _itemsCount, ScriptTokens& tokens) {
            //printf("(%d) TriggerBlock::Set----------------- triggerblock statement item count:%d %s\n",tokens.currIndex, _itemsCount, tokens.Current().ToString().c_str());
            
            itemsCount = _itemsCount;
            items = new StatementBlock[_itemsCount];
            
           // printf("see if we come here\n");
            for (int i=0;i<_itemsCount;i++) {
                if (tokens.SkipIgnoresAndEndIf() == false) {
                    printf("SERIOUS ERROR - reached end\n");
                    break;
                }
                
                items[i].Set(tokens);
            }
        }

        ScriptBlock::ScriptBlock() : triggerBlocks(nullptr), triggerBlockCount(0) { }
        ScriptBlock::~ScriptBlock()
        {
            delete[] triggerBlocks;
            triggerBlocks = nullptr;
        }

        void ScriptBlock::Set(ScriptTokens& tokens) {
            
            triggerBlockCount = tokens.rootBlockCount;
            triggerBlocks = new TriggerBlock[tokens.rootBlockCount];
            
            tokens.currIndex = 0;
            int i = 0;
            // i increments only when we actually assign a trigger block
            // tokens.currIndex increments for every token we process or skip
            while (i < tokens.rootBlockCount && tokens.currIndex < tokens.count) { // tokens.currIndex is incremented elsewhere and is included here as a out of bounds failsafe
                
                ScriptToken& token = tokens.Current();
                
                if (token.type == ScriptTokenType::On)
                {
                    TriggerBlock& triggerBlock = triggerBlocks[i++]; // consume a trigger block
                    //printf("\n(%d) FOUND ON TOKEN\n", tokens.currIndex);
                    tokens.currIndex++; // consume the On token as it dont have any important data
                    
                    ScriptToken& triggerSourceToken = tokens.GetNextAndConsume();//.items[tokens.currIndex++]; // get and consume
                    if (triggerSourceToken.EqualsIC("eachloop")) {
                        triggerBlock.event = new ReactiveEvent(TriggerBlock::AllwaysRun); // using special case of ReactiveEvent
                    }
                    else
                    {
                        HALOperationResult res = DeviceManager::GetDeviceEvent(triggerSourceToken, &triggerBlock.event);
                        if (res != HALOperationResult::Success) {
                           triggerBlock.event = new ReactiveEvent(TriggerBlock::NeverRun); // using special case of ReactiveEvent
                        }
                    }
                    //ReportTokenInfo(tokens.Current(), "this should be a then token: ", tokens.Current().ToString().c_str());
                    int itemCount = tokens.Current().itemsInBlock;
                    tokens.currIndex++; // consume the then
                    triggerBlock.Set(itemCount, tokens); // get number of items and consume
                }
                else if (token.type == ScriptTokenType::If) 
                {
                    TriggerBlock& triggerBlock = triggerBlocks[i++]; // consume a trigger block
                    //printf("\n(%d) FOUND IF TOKEN\n", tokens.currIndex);
                    // here we dont consume anything just pass 
                    // wrap root-level if into a trigger block that always runs
                    triggerBlock.event = new ReactiveEvent(TriggerBlock::AllwaysRun); // using special case of ReactiveEvent
                    triggerBlock.Set(1, tokens);
                }
                else {
                    //printf("\n(%d) SKIPPING TOKEN: %s\n", tokens.currIndex, token.ToString().c_str());
                    tokens.currIndex++;
                    
                }
            }
            if (i != tokens.rootBlockCount) {
                GlobalLogger.Error(F("i != tokens.rootBlockCount"));
            }
        }

        HALOperationResult TriggerBlock::Exec() {
            for (int i=0;i<itemsCount;i++) {
                StatementBlock& statementItem = items[i];
                if (statementItem.handler == nullptr) {
                    printf("\nERRORERRORERRORERRORERRORERRORERRORERRORERRORERROR statementItem.handler == nullptr\n");
                    break;
                }
                HALOperationResult res = statementItem.handler(statementItem.context);
                if (res != HALOperationResult::Success) {
                    return res; // direct return on any failure here
                }
            }
            return HALOperationResult::Success;
        }

        void ScriptBlock::Exec() {
            for (int i=0;i<triggerBlockCount;i++) {
                if (triggerBlocks[i].event == nullptr) {
                    GlobalLogger.Error(F("triggerBlocks[i].event == nullptr"));
                } else {
                    if (triggerBlocks[i].event->CheckForEvent() == false) {
                        continue;
                    }
                }
                HALOperationResult res = triggerBlocks[i].Exec();
                if (res != HALOperationResult::Success) {
                    GlobalLogger.Error(F("trigger: "), HALOperationResultToString(res));
//#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
                    printf("script exec error:%s\n", HALOperationResultToString(res));
//#endif
                }
            }
        }


    }
}