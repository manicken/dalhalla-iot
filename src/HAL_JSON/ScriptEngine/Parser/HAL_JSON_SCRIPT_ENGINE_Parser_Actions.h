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

#include "../HAL_JSON_SCRIPT_ENGINE_Script_Token.h"

namespace HAL_JSON {
    namespace ScriptEngine {
        namespace Parser {
            namespace Actions {
                struct AssignmentParts {
                    ScriptToken lhs;
                    char op;      // assignment operators first char is enough (e.g. "=", "+=", "<<=")
                    ScriptTokens rhs;

                    inline void Clear() {
                        lhs = {};
                        rhs.count = 0;
                        rhs.items = nullptr;
                        rhs.currIndex = 0;
                        rhs.firstTokenStartOffset = nullptr;
                        rhs.rootBlockCount = 0;
                    }
                };
                /** merge actions so that each action 'line'
                 *  is in one token for easier parse 
                 *  this is a variant to MergeActions but
                 *  allows the use of \ to make multiline spanning actions
                 */
                bool MergeActions(ScriptTokens& tokens);
                
                bool VerifyActionBlocks(ScriptTokens& tokens);

                /** used by ActionStatement and Parser_Tests */
                AssignmentParts* ExtractAssignmentParts(ScriptTokens& _tokens);
            }
        }
    }
}