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

#include "DALHAL_SCRIPT_ENGINE_UnconditionalBranch.h"

#include <DALHAL/Support/DALHAL_Logger.h>

#include <DALHAL/ScriptEngine/Parser/DALHAL_SCRIPT_ENGINE_Script_Token.h>
#include <DALHAL/ScriptEngine/Runtime/DALHAL_SCRIPT_ENGINE_StatementBlock.h>

namespace DALHAL {
    namespace ScriptEngine {

        UnconditionalBranch::UnconditionalBranch() : BranchBlock()
        {

        }
        bool UnconditionalBranch::Set(ScriptTokens& tokens) {
#if defined(ESP32) == false && defined(ESP8266) == false
            printf("(%d) UnconditionalBranch::UnconditionalBranch: %s\n", tokens.currIndex, tokens.Current().ToString().c_str());
#endif
            const ScriptToken& elseToken = tokens.GetNextAndConsume();//.items[tokens.currIndex++]; // get and consume

            itemsCount = elseToken.itemsInBlock;
            items = new StatementBlock[itemsCount];

            for (int i=0;i<itemsCount;i++) {
                if (tokens.SkipIgnoresAndEndIf() == SkipTokenResult::ReachedEnd) {
                    GlobalLogger.Error(F("SERIOUS ERROR - reached end\n"));
                    return false;
                }
                items[i].Set(tokens);
            }
            return true;
        }
        UnconditionalBranch::~UnconditionalBranch() {

        }

    }
}