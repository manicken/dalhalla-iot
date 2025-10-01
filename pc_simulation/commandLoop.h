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

#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <chrono>
#include "../src/HAL_JSON/HAL_JSON_ZeroCopyString.h"
#include "../src/HAL_JSON/HAL_JSON_CommandExecutor.h"
#include "../src/HAL_JSON/ScriptEngine/Parser/HAL_JSON_SCRIPT_ENGINE_Parser.h"
#include "../src/HAL_JSON/ScriptEngine/Parser/HAL_JSON_SCRIPT_ENGINE_Parser_Expressions.h"
#include "../src/HAL_JSON/ScriptEngine/Parser/HAL_JSON_SCRIPT_ENGINE_Script_Token.h"
#include "../src/HAL_JSON/ScriptEngine/HAL_JSON_SCRIPT_ENGINE.h"

extern std::atomic<bool> running;

void commandLoop();
void parseCommand(const char* cmd, bool oneShot = false);
