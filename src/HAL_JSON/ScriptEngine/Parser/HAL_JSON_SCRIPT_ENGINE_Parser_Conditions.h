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

#include "HAL_JSON_SCRIPT_ENGINE_Script_Token.h"

namespace HAL_JSON {
    namespace ScriptEngine {
        namespace Parser {
            namespace Conditions {

                /** used by VerifyBlocks */
                int Count_IfTokens(ScriptTokens& tokens);

                /** verify if/on blocks so that they follow the structure
                 * on/if <trigger>/<condition> do/then <action(s)> endon/endif
                 * it also verify that on blocks do only contain if blocks
                 */
                bool VerifyBlocks(ScriptTokens& tokens);

                int CountConditionTokens(ScriptTokens& tokens, int start);
                /** 
                 * merge Conditions into one token for easier parse,
                 * if a AND/OR token is found they are 
                 * replaced by && and || respective 
                 */
                bool MergeConditions(ScriptTokens& tokens);
                bool VerifyConditionBlocks(ScriptTokens& tokens);

                bool EnsureBlocksContainItems(ScriptTokens& _tokens);
                /** this is used together with EnsureBlocksContainItems */
                void CountBlockItems(ScriptTokens& tokens);
            }
        }
    }
}