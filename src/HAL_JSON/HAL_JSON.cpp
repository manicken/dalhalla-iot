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

#include "../Support/Logger.h"
//#include "../Support/ConvertHelper.h"

//#include "HAL_JSON_Device_GlobalDefines.h"

#include "HAL_JSON_Manager.h"
#include "HAL_JSON_REST.h"

#include "ScriptEngine/HAL_JSON_SCRIPT_ENGINE.h"
//#include "ScriptEngine/HAL_JSON_SCRIPT_ENGINE_Parser.h"
#include "ScriptEngine/HAL_JSON_SCRIPT_ENGINE_Script.h"


#include "../Support/Info.h"


#include "HAL_JSON_CommandExecutor.h"

#include "HAL_JSON.h"

namespace HAL_JSON {
    void begin() {
        HAL_JSON::REST::setupRest();
        printf("\nBefore Manager::setupMgr()\n");
        Info::PrintHeapInfo();
        
        if (Manager::setupMgr() == false)
            return;
        printf("\nBefore ScriptEngine::ValidateAndLoadAllActiveScripts\n");
        Info::PrintHeapInfo();
        ScriptEngine::ValidateAndLoadAllActiveScripts(); 
        
        printf("\nAfter ScriptEngine::ValidateAndLoadAllActiveScripts\n");
        Info::PrintHeapInfo();
    }


    long lastmillis = 0;
    void loop() {

        // process REST queue
        while (true) {
            CommandExecutor_LOCK_QUEUE();
            if (CommandExecutor::g_pending.empty()) {
                CommandExecutor_UNLOCK_QUEUE();
                break;
            }
            PendingRequest pr = std::move(CommandExecutor::g_pending.front());
            CommandExecutor::g_pending.pop();
            CommandExecutor_UNLOCK_QUEUE();

            std::string msg;
            ZeroCopyString zcCmd(pr.command.c_str());
            bool ok = CommandExecutor::execute(zcCmd, msg);

            if (pr.cb)
                pr.cb(msg);
            //if (pr.request->client()->connected()) {
            //    pr.request->send(ok ? 200 : 500, "application/json", msg.c_str());
            //}
        }

        long currmillis = millis();
        if (currmillis-lastmillis > 100) {
            lastmillis = currmillis;
            Manager::loop();
            if (ScriptEngine::ScriptsBlock::running)
                ScriptEngine::Exec(); // runs the scriptengine
        }
    }
}