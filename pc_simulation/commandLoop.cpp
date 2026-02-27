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

#include "commandLoop.h"
#include "../src/DALHAL/ScriptEngine/Parser/DALHAL_SCRIPT_ENGINE_Parser_Tests.h"

#include "../src/DALHAL/Devices/HomeAssistant/DALHAL_HA_DeviceDiscovery.h"
#include "../src/DALHAL/Devices/HomeAssistant/DALHAL_HA_CountingPubSubClient.h"
#include "../src/DALHAL/Devices/HomeAssistant/DALHAL_HA_TopicBasePath.h"

std::atomic<bool> running{true};


void ParseHelpCommand(DALHAL::ZeroCopyString& zcCmd) {
    DALHAL::ZeroCopyString zcCmdHelp = zcCmd.SplitOffHead('/');
    if (zcCmdHelp == "hal") {
        DALHAL::ZeroCopyString zcCmdHelpHal = zcCmd.SplitOffHead('/');
        if (zcCmdHelpHal == "read") {
            std::cout << "Format: hal/read/<uint32/bool/float/string>/<deviceUID>/<optional cmd>\n";
        } else if (zcCmdHelpHal == "write") {
            std::cout << "Format: hal/write/<uint32/bool/float/string>/<deviceUID>/<value>\n";
        } else if (zcCmdHelpHal == "reloadcfg") {
            std::cout << "Format: hal/reloadcfg/<optional filename>\n";
        } else {
            std::cout << "Available commands: read, write, reloadcfg\n";
        }
    } else if (zcCmdHelp == "exit") {
        std::cout << "exits this program\n";
    } else if (zcCmdHelp == "status") {
        std::cout << "show program status\n";
    } else if (zcCmdHelp == "help") {
        std::cout << "shows this help\n";
    } else if (zcCmdHelp.NotEmpty()) {
        std::cout << "Unknown help chapter: " << zcCmdHelp.ToString() << "\n";
        std::cout << "Available chapters: exit, status, help, hal\n";
    } else {
        std::cout << "Available commands: exit, status, help, hal\n";
    }
}
void exprTestLoad(DALHAL::ZeroCopyString& zcStr) {
    DALHAL::ZeroCopyString zcFilePath = zcStr.SplitOffHead('/');
    std::string strFilePath = zcFilePath.ToString();
    char* contents = nullptr;
    const char* cFilePath = strFilePath.c_str();
    LittleFS_ext::FileResult fileResult = LittleFS_ext::load_text_file(cFilePath, &contents);
    if (fileResult != LittleFS_ext::FileResult::Success) {
        if (fileResult == LittleFS_ext::FileResult::FileNotFound)
            std::cout << "Error: file could not be found: " << strFilePath << "\n";
        else if (fileResult == LittleFS_ext::FileResult::FileEmpty)
            std::cout << "Error: file empty: " << strFilePath << "\n";
        else
            std::cout << "Error: other file error: " << strFilePath << "\n";
    } 
    DALHAL::ScriptEngine::ScriptTokens tokens;
    DALHAL::ScriptEngine::ScriptToken token(contents);

    tokens.count = 1;
    tokens.items = &token;
    bool valid = DALHAL::ScriptEngine::Expressions::ValidateExpression(tokens);
    if (valid) {
        std::cout << "Parse [OK]\n";
    }
    delete[] contents;
}
void parseCommand(const char* cmd, bool oneShot) {
    DALHAL::ZeroCopyString zcCmd(cmd);

    DALHAL::ZeroCopyString zcCmdRoot = zcCmd.SplitOffHead('/');

    if (zcCmdRoot == "exit" || zcCmdRoot == "e") {
        std::cout << "Shutting down...\n";
        running = false;
    } else if (zcCmdRoot == "status") {
        std::cout << "Status: running\n";
    } else if (zcCmdRoot == "help") {
        ParseHelpCommand(zcCmd);            
    } else if (zcCmdRoot == "hal") {
        DALHAL::CommandExecutor::execute(zcCmd, [](const std::string& response){ 
            std::cout << response << "\n"; 
        });

    } else if (zcCmdRoot == "expr") {
        exprTestLoad(zcCmd);
    } else if (zcCmdRoot == "loadrules" || zcCmdRoot == "lr") {
        if (oneShot) {
            DALHAL::DeviceManager::setupMgr();
        }
        DALHAL::ZeroCopyString zcFilePath = zcCmd.SplitOffHead('/');
        std::cout << "using rule set file:" << zcFilePath.ToString() << "\n";
        std::string filePath;
        if (zcFilePath.NotEmpty())
            filePath = zcFilePath.ToString();
        else
            filePath = "ruleset.txt";
        DALHAL::ScriptEngine::Expressions::CalcStackSizesInit();
        auto start = std::chrono::high_resolution_clock::now();
        DALHAL::ScriptEngine::Parser::ReadAndParseScriptFile(filePath.c_str(), nullptr);
        DALHAL::ScriptEngine::Expressions::PrintCalcedStackSizes();
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start;

        std::cout << "Parse time: " << duration.count() << " ms\n";
    } else if (zcCmdRoot == "ldex") {
        if (oneShot) {
            //DALHAL::DeviceManager::setup(); // could use this later on
        }
        auto start = std::chrono::high_resolution_clock::now();
        DALHAL::ZeroCopyString zcFilePath = zcCmd.SplitOffHead('/');
        std::cout << "using expression to RPN conv file:" << zcFilePath.ToString() << "\n";
        std::string filePath = zcFilePath.ToString();
        DALHAL::ScriptEngine::Parser::Tests::ParseExpressionTest(filePath.c_str());
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start;

        std::cout << "Parse time: " << duration.count() << " ms\n";
    } else if (zcCmdRoot == "ldac") {
        if (oneShot) {
            //DALHAL::DeviceManager::setup(); // could use this later on
        }
        auto start = std::chrono::high_resolution_clock::now();
        DALHAL::ZeroCopyString zcFilePath = zcCmd.SplitOffHead('/');
        std::cout << "using action expression to RPN conv file:" << zcFilePath.ToString() << "\n";
        std::string filePath = zcFilePath.ToString();
        DALHAL::ScriptEngine::Parser::Tests::ParseActionExpressionTest(filePath.c_str());
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start;

        std::cout << "Parse time: " << duration.count() << " ms\n";
    } else if (zcCmdRoot == "ldtest") {
        auto start = std::chrono::high_resolution_clock::now();
        // loads the json HAL config, 
        // this is needed as it's used by the script validator
        DALHAL::DeviceManager::setupMgr();
        DALHAL::ScriptEngine::ValidateAndLoadAllActiveScripts();
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start;

        std::cout << "Parse time: " << duration.count() << " ms\n";
    } else if (zcCmdRoot == "ldval") {
        auto start = std::chrono::high_resolution_clock::now();
        // loads the json HAL config, 
        // this is needed as it's used by the script validator
        DALHAL::DeviceManager::setupMgr();
        DALHAL::ScriptEngine::ScriptsToLoad scriptsToLoad;
        DALHAL::ScriptEngine::ValidateAllActiveScripts(scriptsToLoad);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start;

        std::cout << "Parse time: " << duration.count() << " ms\n";
    } else if (zcCmdRoot == "ldcfg") {
        auto start = std::chrono::high_resolution_clock::now();
        DALHAL::DeviceManager::setupMgr();
        
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start;

        std::cout << "Parse time: " << duration.count() << " ms\n";
    } else if (zcCmdRoot == "ha_test") {
        DALHAL::CountingPubSubClient cntPSC;
/*
#define JSON(...) #__VA_ARGS__
        const char* deviceGroupUIDstr = "deviceGroupUIDstr";
        const char* nameStr = "jsonObjDeviceGroup_name";
        const char* jsonFmt = JSON(
        {
            "device": {
            "identifiers": ["%s"],
            "manufacturer": "Dalhal",
            "model": "Virtual Sensor",
            "name": "%s"
            }
        },
        );
        DALHAL::PSC_JsonWriter::printf_str(cntPSC, jsonFmt, deviceGroupUIDstr, nameStr);
*/
        StaticJsonDocument<512> deviceGroupDoc;
        deviceGroupDoc["uid"] = "grp001";
        deviceGroupDoc["name"] = "Test Group";
        deviceGroupDoc["manufacturer"] = "Custom Maker";
        deviceGroupDoc["model"] = "Model X";

        StaticJsonDocument<512> deviceDoc;
        deviceDoc["type"] = "sensor";
        deviceDoc["uid"] = "temp01";
        deviceDoc["name"] = "Temperature Sensor";
        //deviceDoc["icon"] = "iconpath";
        deviceDoc["some number"] = 5;
        deviceDoc["some number2"] = 5.2f;
        deviceDoc["use_availability_topic"] = true;
        deviceDoc["payload_available"] = "on";
        deviceDoc["payload_not_available"] = "off";

        std::cout << "\nDALHAL::PSC_JsonWriter::SendAllItems:\n";
        DALHAL::PSC_JsonWriter::SendAllItems(cntPSC, deviceDoc);
        std::cout << "\n";
        DALHAL::TopicBasePath topicBasePath;
        topicBasePath.Set("PC sim test", "sim_test_uid");
        // Call the function
        const char* type_cStr = deviceDoc["type"];
        const char* uid_cStr =  deviceDoc["uid"];
        const char* deviceID_cStr = "PC_sim";
        const char* cfgTopic_cStr = DALHAL::HA_DeviceDiscovery::GetDiscoveryCfgTopic(deviceID_cStr, type_cStr, uid_cStr);
        DALHAL::HA_DeviceDiscovery::SendDiscovery(cntPSC, deviceID_cStr, cfgTopic_cStr, deviceDoc, deviceGroupDoc, topicBasePath);
    
        std::cout << std::endl;

    } else {
        std::cout << "Unknown command: " << cmd << "\n";
    }
}
void commandLoop() {
    std::string cmd;
    std::cin.clear();  // clear error/EOF flags
    while (running) {
        std::cout << "> ";
        if (!std::getline(std::cin, cmd)) {
            if (std::cin.eof()) {
                std::cout << "EOF reached, exiting command loop.\n"; //
            } else if (std::cin.fail()) {
                std::cout << "Input stream failure detected, exiting.\n";
            } else if (std::cin.bad()) {
                std::cout << "Input stream bad state, exiting.\n";
            } else {
                std::cout << "other error\n" << std::flush;
            }
            std::cout << "Shutting down...\n" << std::flush; // this never get printed
            running = false; // will also signal to the main loop to exit
        } 
        else
            parseCommand(cmd.c_str(), false);
    }
}