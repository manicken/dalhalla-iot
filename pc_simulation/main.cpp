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

    // this file is for testing Rule Engine on PC environment

    #include <iostream>
    #include <string>
    #include <cstdint>
    #include <fstream>
    #include <vector>
    #include <stack>
    #include <string_view>

    #include "../src/HAL_JSON/HAL_JSON_Manager.h"
    #include "../src/HAL_JSON/ScriptEngine/HAL_JSON_SCRIPT_ENGINE.h"
#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__) // use this to avoid getting vscode error here
    #include "stubs/HAL_JSON_REST/HAL_JSON_REST.h"
#endif
    #include "../src/Support/ConvertHelper.h"
    #include "../src/Support/CharArrayHelpers.h"
    #include "../src/HAL_JSON/HAL_JSON_ZeroCopyString.h"
     #include <ArduinoJson.h>
    #include "commandLoop.h"

    std::string halJsonRestCallback(const std::string& path) {
        HAL_JSON::ZeroCopyString zcCmd(path.c_str()+1); // +1 = remove leading /
        std::string message;
        HAL_JSON::CommandExecutor::execute(zcCmd, message);
        return message;
    }

    int main(int argc, char* argv[]) {
        int test = 5;
        int test2 = 0-1 * test;
        test2 *= 1;
        std::cout << "********************************************************************" << std::endl;
#if defined(_WIN32)
        std::cout << "* Dalhalla IoT development simulator - Running on Windows (MinGW)  *" << std::endl;
#elif defined(__linux__)
        std::cout << "* Dalhalla IoT development simulator - Running on Linux            *" << std::endl;
#endif
        std::cout << "********************************************************************" << std::endl;

        if (argc > 1) {
            // one shot tests
            parseCommand(argv[1], true); // true mean one short test
            HAL_JSON::Manager::CleanUp();
            return 0;
        }
        
        std::cout << "\n****** Starting REST api server:\n";
#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__) // use this to avoid getting vscode error here
        //HAL_JSON::REST::setup(halJsonRestCallback); // this will start the server
#endif
        std::cout << "\n****** Init HAL_JSON Manager\n";
        HAL_JSON::Manager::setupMgr();
        HAL_JSON::ScriptEngine::ValidateAndLoadAllActiveScripts();
        std::cout << "\n****** Starting commandLoop thread\n";
        std::thread cmdThread(commandLoop); // start command input thread from commandLoop that is in commandLoop.h
        long lastmillis = 0;
        while (running) { // running is in commandLoop.h
            HAL_JSON::Manager::loop();
            long currmillis = millis();
            if (currmillis-lastmillis > 100) {
                lastmillis = currmillis;
                if (HAL_JSON::ScriptEngine::ScriptsBlock::running)
                    HAL_JSON::ScriptEngine::Exec(); // runs the scriptengine
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        cmdThread.join(); // wait for command thread to finish
        std::cout << "Exited cleanly.\n" << std::flush;
        HAL_JSON::Manager::CleanUp();
        return 0;
    }