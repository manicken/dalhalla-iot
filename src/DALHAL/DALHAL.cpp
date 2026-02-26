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

#include <Arduino.h>
#include <ArduinoJson.h>

#include <stdlib.h>

#include "Support/DALHAL_Logger.h"
#include "Core/DALHAL_Manager.h"
#include "API/DALHAL_API.h"

#include "ScriptEngine/DALHAL_SCRIPT_ENGINE.h"
#include "../System/Info.h"

#include "DALHAL.h"

namespace DALHAL {
    void begin() {
        //DALHAL::REST::setupRest();
        DALHAL::WebSocketAPI::setup();
        //printf("\nBefore Manager::setupMgr()\n");
        //Info::PrintHeapInfo();
        
        if (Manager::setupMgr() == false)
            return;
        //printf("\nBefore ScriptEngine::ValidateAndLoadAllActiveScripts\n");
        //Info::PrintHeapInfo();
        ScriptEngine::ValidateAndLoadAllActiveScripts(); 
        
        //printf("\nAfter ScriptEngine::ValidateAndLoadAllActiveScripts\n");
        //Info::PrintHeapInfo();
    }


    long lastmillis = 0;
    void loop() {

        // process Async Requests queue
        while (true) {
            CommandExecutor_LOCK_QUEUE();
            if (CommandExecutor::g_pending.empty()) {
                CommandExecutor_UNLOCK_QUEUE();
                break;
            }
            PendingRequest pr = std::move(CommandExecutor::g_pending.front());
            CommandExecutor::g_pending.pop();
            CommandExecutor_UNLOCK_QUEUE();

            ZeroCopyString zcCmd(pr.command.c_str());
            bool ok = CommandExecutor::execute(zcCmd, pr.cb);
        }

        long currmillis = millis();
        if (currmillis-lastmillis > 100) {
            lastmillis = currmillis;
            Manager::loop();
            
            if (ScriptEngine::ScriptsBlock::running)
                ScriptEngine::Exec(); // runs the scriptengine
        }
        WebSocketAPI::loop();
        SerialAPI::loop();
    }
}