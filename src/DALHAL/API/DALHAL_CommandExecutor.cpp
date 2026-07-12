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

#include "DALHAL_CommandExecutor.h"

#include <DALHAL/Support/DALHAL_MACROS.h>

#include <DALHAL/Support/DALHAL_Logger.h>

#include <BUILD_INFO.h>
#include <DALHAL/Support/base64.h>

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>

#include <DALHAL/Core/Manager/DALHAL_GPIO_Manager.h>
#include <DALHAL/Core/Manager/DALHAL_DeviceManager.h>
#include <DALHAL/Core/Types/DALHAL_Registry.h>

#include <DALHAL/Core/Types/DALHAL_DeviceFunctionTable.h>

#include <DALHAL/Devices/_Registry/DALHAL_DevicesRegistry.h>
#include <DALHAL/ScriptEngine/DALHAL_SCRIPT_ENGINE.h>
#if defined(ESP8266) || defined(ESP32)
#include <System/Info.h>

#endif

#include <DALHAL/Drivers/HearbeatLed.h>
#include <Scheduler/Scheduler.h>

//#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_ToJsonString.h>
#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_ToJsonString.h>

#include <DALHAL/API/DALHAL_BlockStreamer.h>
#include <DALHAL/API/DALHAL_StringBuilderStreamer.h>

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif

#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
#include <iostream>
#endif

namespace DALHAL {

#if defined(ESP32)
    portMUX_TYPE CommandExecutor::g_pendingMux = portMUX_INITIALIZER_UNLOCKED;
#elif defined(ESP8266)

#elif defined(_WIN32) || defined(__linux__) || defined(__MAC__)
    std::mutex CommandExecutor::g_pendingMutex;
#else

#endif
    std::queue<PendingRequest> CommandExecutor::g_pending;

    // Command frame parsing is intentionally layered to preserve routing integrity.
    //
    // Format:
    //   <type>/<uid>#<cmd>/<params>
    //
    // Parsing rules:
    // 1. First '/' splits transport layer (type) from the rest.
    // 2. Second '/' splits device routing layer (uid + possible cmd/params).
    // 3. '#' is ONLY valid inside the routing segment and separates UID from function name.
    // 4. Everything after '#' is treated as opaque function parameters.
    //
    // Important design rule:
    // - '/' defines protocol boundaries (scoped split order)
    // - '#' defines execution boundary (function dispatch)
    // - parameters are never parsed at routing level to avoid delimiter collisions
    //
    // This ordering ensures payload content cannot corrupt UID or command resolution.
    CommandExecutor::ReadWriteCmdParameters::ReadWriteCmdParameters(ZeroCopyString& zcStr) {
        zcStr.Trim();
        if (zcStr.StartsWith('?')) {
            isHelpRequest = true;
            zcStr.start++;
        } else {
            isHelpRequest = false;
        }
        zcUid = zcStr.SplitOffHead('/');
        zcUid.Trim();
        const char* cmdSeparator = zcUid.FindCharReverse('#');
        if (cmdSeparator != nullptr) {
            zcCmd = zcUid.SplitOffTail(cmdSeparator);
            zcCmd.Trim();
            cmdIsPresent = true;
        } else {
            cmdIsPresent = false;
        }
        const char* startBracket = zcUid.FindChar('[');
        const char* endBracket = zcUid.FindChar(']');
        if (startBracket != nullptr) { // do not really care if end bracket is found or not actually we only need the first to make the split
            zcBracketParameter = zcUid.SplitOffTail(startBracket);
            if (endBracket != nullptr) {
                zcBracketParameter.end = endBracket;
            }
            zcBracketParameter.Trim();
            zcUid.Trim();
            isBracketOp = true;
        } else {
            isBracketOp = false;
        }
        zcParameters = zcStr; // the value is the rest
        zcParameters.Trim();
    }

    CommandExecutor::ExecCmdParameters::ExecCmdParameters(ZeroCopyString& zcStr) {
        zcStr.Trim();
        if (zcStr.StartsWith('?')) {
            isHelpRequest = true;
            zcStr.start++;
        }
        zcUid = zcStr.SplitOffHead('/');
        zcUid.Trim();
        const char* cmdSeparator = zcUid.FindCharReverse('#');
        if (cmdSeparator != nullptr) {
            zcCmd = zcUid.SplitOffTail(cmdSeparator);
            zcCmd.Trim();
            cmdIsPresent = true;
        } else {
            cmdIsPresent = false;
        }
        // here are not any parameters
    }

#define DALHAL_CMD_CHILDREN(n) n, DALHAL_ARRAY_COUNT(n)
#define DALHAL_CMD_EXEC_ENTRY(name, exec, help) { CE_MATCH_EMIT_STR(name), exec, CE_EMIT_STR(help) }
#define DALHAL_CMD_GROUP_ENTRY(name, childs, help) { CE_MATCH_EMIT_STR(name), DALHAL_CMD_CHILDREN(childs), CE_EMIT_STR(help) }
#define DALHAL_CMD_EXEC_AND_GROUP_ENTRY(name, exec, childs, help) { CE_MATCH_EMIT_STR(name), exec, DALHAL_CMD_CHILDREN(childs), CE_EMIT_STR(help) }

#define DALHAL_CMD_EXEC_ENTRY_WFLAG(name, exec, flags, help) { CE_MATCH_EMIT_STR(name), exec, flags, CE_EMIT_STR(help) }
#define DALHAL_CMD_GROUP_ENTRY_WFLAG(name, childs, flags, help) { CE_MATCH_EMIT_STR(name), DALHAL_CMD_CHILDREN(childs), flags, CE_EMIT_STR(help) }
#define DALHAL_CMD_EXEC_AND_GROUP_ENTRY_WFLAG(name, exec, childs, flags, help) { CE_MATCH_EMIT_STR(name), exec, DALHAL_CMD_CHILDREN(childs), flags, CE_EMIT_STR(help) }

    bool ConnectToNewWiFi(const char* ssid, const char* pass);
    bool reloadJSON(ZeroCopyString& zcStr, CommandCallback cb);

    HALOperationResult Exec_Scheduler_Cmd(ZeroCopyString& zcStr, CommandCallback cb); // TODO to be implemented as a device

    HALOperationResult Exec_Help_Cmd_All(ZeroCopyString& zcStr, CommandCallback cb);
    HALOperationResult Exec_Help_Cmd_Type(ZeroCopyString& zcStr, CommandCallback cb);
    HALOperationResult Exec_Help_Cmd_DeviceInstance(ZeroCopyString& zcStr, CommandCallback cb);
    HALOperationResult Exec_Help_Cmd_By_Path(ZeroCopyString& zcStr, CommandCallback cb);
    

    HALOperationResult Exec_System_Info(ZeroCopyString& zcStr, CommandCallback cb);
    HALOperationResult Exec_System_Heap(ZeroCopyString& zcStr, CommandCallback cb);
    HALOperationResult Exec_System_Reset_Restart(ZeroCopyString& zcStr, CommandCallback cb);
    HALOperationResult Exec_System_HeartbeatLed(ZeroCopyString& zcStr, CommandCallback cb);
    HALOperationResult Exec_System_Build_Ver_Print(ZeroCopyString& zcStr, CommandCallback cb);
    HALOperationResult Exec_WiFi_Scan(ZeroCopyString& zcStr, CommandCallback cb);
    HALOperationResult Exec_WiFi_Set_b64(ZeroCopyString& zcStr, CommandCallback cb);
    HALOperationResult Exec_WiFi_Set_Json(ZeroCopyString& zcStr, CommandCallback cb);

    HALOperationResult Exec_Hal_Exec(ZeroCopyString& zcStr, CommandCallback cb);
    HALOperationResult Exec_Hal_Read_String(ZeroCopyString& zcStr, CommandCallback cb);
    HALOperationResult Exec_Hal_Read_Value(ZeroCopyString& zcStr, CommandCallback cb); // to make explicit type functions obsolete

    HALOperationResult Exec_Hal_Write_String(ZeroCopyString& zcStr, CommandCallback cb);
    HALOperationResult Exec_Hal_Write_Value(ZeroCopyString& zcStr, CommandCallback cb);  // to make explicit type functions obsolete

    HALOperationResult Exec_Hal_Config_Reload(ZeroCopyString& zcStr, CommandCallback cb);
    HALOperationResult Exec_Hal_Config_Unload(ZeroCopyString& zcStr, CommandCallback cb);

    HALOperationResult Exec_Hal_Scripts_Reload(ZeroCopyString& zcStr, CommandCallback cb);
    HALOperationResult Exec_Hal_Scripts_Unload(ZeroCopyString& zcStr, CommandCallback cb);
    HALOperationResult Exec_Hal_Scripts_Stop(ZeroCopyString& zcStr, CommandCallback cb);
    HALOperationResult Exec_Hal_Scripts_Start(ZeroCopyString& zcStr, CommandCallback cb);

    HALOperationResult Exec_Hal_GetAvailableGPIOs(ZeroCopyString& zcStr, CommandCallback cb);
    HALOperationResult Exec_Hal_PrintDevices(ZeroCopyString& zcStr, CommandCallback cb);
    HALOperationResult Exec_PrintLog(ZeroCopyString& zcStr, CommandCallback cb);
    HALOperationResult Exec_Hal_PrintRegistry_Types(ZeroCopyString& zcStr, CommandCallback cb);
    HALOperationResult Exec_Hal_PrintRegistry_Functions(ZeroCopyString& zcStr, CommandCallback cb);
    HALOperationResult Exec_Hal_PrintRegistry_Events(ZeroCopyString& zcStr, CommandCallback cb);
    HALOperationResult Exec_Hal_PrintJsonSchemas(ZeroCopyString& zcStr, CommandCallback cb);

    static constexpr CommandNode HalWriteItems[] = {
        DALHAL_CMD_EXEC_ENTRY("string", Exec_Hal_Write_String, "hal write string"),
        DALHAL_CMD_EXEC_ENTRY("str", Exec_Hal_Write_String, "hal write string - short alias for string"),
        DALHAL_CMD_EXEC_ENTRY("value", Exec_Hal_Write_Value, "hal write numeric value"),
        DALHAL_CMD_EXEC_ENTRY("val", Exec_Hal_Write_Value, "hal write numeric value - short alias for value"),
    };

    static constexpr CommandNode HalReadItems[] = {
        DALHAL_CMD_EXEC_ENTRY("string", Exec_Hal_Read_String, "hal read string"),
        DALHAL_CMD_EXEC_ENTRY("str", Exec_Hal_Read_String, "hal read string - short alias for string"),
        DALHAL_CMD_EXEC_ENTRY("value", Exec_Hal_Read_Value, "hal read numeric value"),
        DALHAL_CMD_EXEC_ENTRY("val", Exec_Hal_Read_Value, "hal read numeric value - short alias for value"),
    };

    static constexpr CommandNode HalScriptItems[] = {
        DALHAL_CMD_EXEC_ENTRY_WFLAG("reload", Exec_Hal_Scripts_Reload, CommandNode::Flags::AUTOGEN_BUTTON, "scripts reload"),
        DALHAL_CMD_EXEC_ENTRY_WFLAG("unload", Exec_Hal_Scripts_Unload, CommandNode::Flags::AUTOGEN_BUTTON, "hal script unload, can be used to provide a clean slate before loading new script"),
        DALHAL_CMD_EXEC_ENTRY_WFLAG("stop", Exec_Hal_Scripts_Stop, CommandNode::Flags::AUTOGEN_BUTTON, "scripts stop/pause execution"),
        DALHAL_CMD_EXEC_ENTRY_WFLAG("start", Exec_Hal_Scripts_Start, CommandNode::Flags::AUTOGEN_BUTTON, "scripts start/resume execution"),
    };

    static constexpr CommandNode HalConfigItems[] = {
        DALHAL_CMD_EXEC_ENTRY_WFLAG("reload", Exec_Hal_Config_Reload, CommandNode::Flags::AUTOGEN_BUTTON, "hal config first validate cfg from file and if success, 1. unloads the current cfg. 2. load new cfg. 3. reload/validate script."),
        DALHAL_CMD_EXEC_ENTRY_WFLAG("unload", Exec_Hal_Config_Unload, CommandNode::Flags::AUTOGEN_BUTTON, "hal config/script unload, this makes the current cfg clean, can be used to make sure that the memory is clean before loading new config"),
        DALHAL_CMD_EXEC_ENTRY_WFLAG("list", Exec_Hal_PrintDevices, CommandNode::Flags::AUTOGEN_BUTTON, "list all current loaded devices"),
    };

    static constexpr CommandNode HalMetaRegistryItems[] = {
        DALHAL_CMD_EXEC_ENTRY_WFLAG("types", Exec_Hal_PrintRegistry_Types, CommandNode::Flags::AUTOGEN_BUTTON, "print device type registry basedata"),
        DALHAL_CMD_EXEC_ENTRY_WFLAG("functions", Exec_Hal_PrintRegistry_Functions, CommandNode::Flags::AUTOGEN_BUTTON, "print device type registry with functions"),
        DALHAL_CMD_EXEC_ENTRY_WFLAG("events", Exec_Hal_PrintRegistry_Events, CommandNode::Flags::AUTOGEN_BUTTON, "print device type registry with events"),
        DALHAL_CMD_EXEC_ENTRY_WFLAG("cfgschema", Exec_Hal_PrintJsonSchemas, CommandNode::Flags::AUTOGEN_BUTTON, "print config json schema"),
    };

    static constexpr CommandNode HalMetaItems[] = {
        DALHAL_CMD_EXEC_ENTRY_WFLAG("gpio", Exec_Hal_GetAvailableGPIOs, CommandNode::Flags::AUTOGEN_BUTTON, "get a list of available GPIO on this target and their functions"),
        DALHAL_CMD_GROUP_ENTRY("reg", HalMetaRegistryItems, "print registry metadata"),
    };

    static constexpr CommandNode HalItems[] = {
        DALHAL_CMD_EXEC_ENTRY("exec", Exec_Hal_Exec, "run device exec cmd"),
        DALHAL_CMD_GROUP_ENTRY("write", HalWriteItems, "run device write cmds"),
        DALHAL_CMD_GROUP_ENTRY("read", HalReadItems, "run device read cmds"),
        DALHAL_CMD_GROUP_ENTRY("wr", HalWriteItems, "run device write cmds - alias for write"),
        DALHAL_CMD_GROUP_ENTRY("rd", HalReadItems, "run device read cmds - alias for read"),
        DALHAL_CMD_GROUP_ENTRY("config", HalConfigItems, "hal config cmds"),
        DALHAL_CMD_GROUP_ENTRY("scripts", HalScriptItems, "script specific commands"),
        DALHAL_CMD_GROUP_ENTRY("meta", HalMetaItems, "print metadata"),
    };
#if defined(ESP8266) || defined(ESP32)
    static constexpr CommandNode WiFiItems[] = {
        DALHAL_CMD_EXEC_ENTRY("scan", Exec_WiFi_Scan, "Scan WiFi for AP:s"),
        DALHAL_CMD_EXEC_ENTRY("set_b64", Exec_WiFi_Set_b64, "Set current WiFi settings from the given b64 encoded string"),
        DALHAL_CMD_EXEC_ENTRY("set_json", Exec_WiFi_Set_Json, "Set current WiFi settings from the given json encoded string"),
    };
#endif
    static constexpr CommandNode SystemItems[] = {
        
        DALHAL_CMD_EXEC_ENTRY("info", Exec_System_Info, "System info"),
#if defined(ESP8266) || defined(ESP32)
        DALHAL_CMD_EXEC_ENTRY_WFLAG("heap", Exec_System_Heap, CommandNode::Flags::AUTOGEN_BUTTON, "Print heap info"),
        DALHAL_CMD_EXEC_ENTRY_WFLAG("reset", Exec_System_Reset_Restart, (CommandNode::Flags::AUTOGEN_BUTTON | CommandNode::Flags::DANGER), "Reset the system"),
        DALHAL_CMD_EXEC_ENTRY_WFLAG("restart", Exec_System_Reset_Restart, (CommandNode::Flags::DANGER), "Restart the system (same as reset)"),
#endif
        DALHAL_CMD_EXEC_ENTRY("HeartbeatLed", Exec_System_HeartbeatLed, "HeartBeatLed set timings"),
        DALHAL_CMD_EXEC_ENTRY("ver", Exec_System_Build_Ver_Print, "Print current build version"),
    };

    static constexpr CommandNode HelpItems[] = {
        DALHAL_CMD_EXEC_ENTRY("all", Exec_Help_Cmd_All, "Returns the whole cmd tree"),
        DALHAL_CMD_EXEC_ENTRY("type", Exec_Help_Cmd_Type, "Show help by a device type"),
        DALHAL_CMD_EXEC_ENTRY("device", Exec_Help_Cmd_DeviceInstance, "Show help for a given device instance path"),
        DALHAL_CMD_EXEC_ENTRY("@", Exec_Help_Cmd_By_Path, "Show help for a given cmd path"),
    };

    static constexpr CommandNode RootItems[] = {
        DALHAL_CMD_GROUP_ENTRY("hal", HalItems, "HAL subsystem"),
        DALHAL_CMD_GROUP_ENTRY("system", SystemItems, "System commands"),
#if defined(ESP8266) || defined(ESP32)
        DALHAL_CMD_GROUP_ENTRY("wifi", WiFiItems, "WiFi management"),
#endif
        DALHAL_CMD_EXEC_ENTRY_WFLAG("printLog", Exec_PrintLog, CommandNode::Flags::AUTOGEN_BUTTON, "print log"),
        DALHAL_CMD_EXEC_ENTRY("schedule", Exec_Scheduler_Cmd, "Schedule commands"), // to be removed in the future as it would be implemented as a device
        DALHAL_CMD_EXEC_AND_GROUP_ENTRY("help", Exec_Help_Cmd_All, HelpItems, "Show help"),
    };

    static constexpr CommandNode RootItem = {
        CE_MATCH_EMIT_STR(""), DALHAL_CMD_CHILDREN(RootItems), CE_EMIT_STR("root item")
    };

    void PrintOperationSuccess(const char* tag, CommandCallback cb) {
        BlockStreamer bs(cb, tag, BlockStreamer::DataType::Json);
        StringBuilderStreamer& sbs = bs.writer();
        sbs.write(F("{\"info\":\"OK\"}"));
    }

    void PrintOperationFail(const char* tag, ZeroCopyString& message, CommandCallback cb) {
        BlockStreamer bs(cb, tag, BlockStreamer::DataType::Json);
        StringBuilderStreamer& sbs = bs.writer();
        sbs.write_json_object_begin();
        sbs.write_jsonMemberStart(F("error"));
        sbs.write_jsonQuoted(message.start, message.Length());
        sbs.write_json_object_end();
    }

    static void PrintHelpNodeJson(const CommandNode& node, StringBuilderStreamer& sbs) {
        sbs.write_json_object_begin();

        // name
        sbs.write_jsonMemberStart(F("name"));
        node.name(nullptr, &sbs);

        // help
        sbs.write_json_value_separator();
        sbs.write_jsonMemberStart(F("help"));
        node.help(sbs);

        if (node.flags & CommandNode::Flags::AUTOGEN_BUTTON) {
            sbs.write_json_value_separator();
            sbs.write_jsonBool(F("autogenbutton"), true);
        }

        if (node.flags & CommandNode::Flags::DANGER) {
            sbs.write_json_value_separator();
            sbs.write_jsonBool(F("autogendanger"), true);
        }

        // children
        sbs.write_json_value_separator();
        sbs.write_jsonMemberStart(F("children"));
        sbs.write_json_array_begin();

        for (size_t i = 0; i < node.children_count; i++) {
            if (node.children[i].flags & CommandNode::Flags::HIDDEN) {
                continue;
            }
            if (i > 0) {
                sbs.write_json_value_separator();
            }

            PrintHelpNodeJson(node.children[i], sbs);
        }

        sbs.write_json_array_end();

        sbs.write_json_object_end();
    }

    HALOperationResult ValidateDeviceExistence(CommandExecutor::ReadWriteCmdParameters& params, BlockStreamer& bs, Device** outDevice) {
        // check if have uid given
        if (params.zcUid.IsEmpty()) {
            StringBuilderStreamer& sbs = bs.writer();
            sbs.write_json_object_begin();
            sbs.write_jsonString(F("error"), F("uid path is empty"));
            sbs.write_json_object_end();
            return HALOperationResult::DeviceUIDPathEmpty;
        }
        // check if device exists
        UIDPath uidPath(params.zcUid);
        DeviceFindResult devFindRes = DeviceManager::findDevice(uidPath, *outDevice);
        if (devFindRes != DeviceFindResult::Success) {
            StringBuilderStreamer& sbs = bs.writer();
            sbs.write_json_object_begin();
            sbs.write_jsonMemberStart(F("error"));
            sbs.write_char('"');
            sbs.write(F("device not found: "));
            sbs.write(params.zcUid);
            sbs.write_char('"');
            sbs.write_json_object_end();
            return HALOperationResult::DeviceNotFound;
        }

        return HALOperationResult::Success;
    }

    HALOperationResult ExecuteNode(const CommandNode& node, ZeroCopyString& zcStr, CommandCallback cb) {
        if (zcStr.IsEmpty()) {
            if (node.execute != nullptr) {
                return node.execute(zcStr, cb);
            }

            return HALOperationResult::UnsupportedCommand;
        }

        ZeroCopyString next = zcStr.SplitOffHead('/');

        for (size_t i = 0; i < node.children_count; i++) {
            const CommandNode& child = node.children[i];

            if (child.name(&next, nullptr)) {

                // If there is more path remaining, descend
                if (zcStr.NotEmpty() && child.children_count > 0) {
                    return ExecuteNode(child, zcStr, cb);
                }

                // Path ended here
                if (child.execute != nullptr) {
                    return child.execute(zcStr, cb);
                }

                // No execution here, but maybe children exist
                if (child.children_count > 0) {
                    return ExecuteNode(child, zcStr, cb);
                }

                return HALOperationResult::UnsupportedCommand;
            }
        }

        // No child matched
        // Allow current node to consume remaining args
        if (node.execute != nullptr) {
            return node.execute(zcStr, cb);
        }

        return HALOperationResult::UnsupportedCommand;
    }

    bool CommandExecutor::execute(ZeroCopyString& zcStr, CommandCallback cb) {
        ZeroCopyString zcStrCmd = zcStr; // create copy to make it possible to print full cmd in case of error

        HALOperationResult res = ExecuteNode(RootItem, zcStr, cb);
        if (res != HALOperationResult::Success) {
            BlockStreamer bs(cb, zcStrCmd.start, BlockStreamer::DataType::Json);
            StringBuilderStreamer& sbs = bs.writer();
            sbs.write_json_object_begin();
            sbs.write_jsonString(F("error"), HALOperationResultToString(res));
            sbs.write_json_value_separator();
            sbs.write_jsonString(F("cmd"), zcStrCmd);
            sbs.write_json_object_end();
            return false;
        } 
        return true;
    }

    bool reloadJSON(ZeroCopyString& zcStr, CommandCallback cb) {
        ZeroCopyString zcOptionalFileName = zcStr.SplitOffHead('/');
#ifdef DALHAL_CommandExecutor_DEBUG_CMD
        message += "\"filename\":\"" + (zcOptionalFileName.NotEmpty()?zcOptionalFileName.ToString():"default") + "\"}";
#endif
        std::string filePath;
#if defined(ESP32) || defined(ESP8266)
        filePath = '/';
#endif
        if (zcOptionalFileName.Length() == 0) {
            filePath += String(F("hal/cfg.json")).c_str();
        } else {
            filePath += String(F("hal/")).c_str() + zcOptionalFileName.ToString();
        }

#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
        std::cout << "Reload cfg json: " << filePath << std::endl;  
#endif
        BlockStreamer bs(cb, "reloadJSON", BlockStreamer::DataType::Json);
        StringBuilderStreamer& sbs = bs.writer();
        if (DeviceManager::ReadJSON(filePath.c_str())) {
            sbs.write(F("{\"info\":\"OK\"}"));
            DeviceManager::begin();
            
            return true;
        } else {
            sbs.write(F("{\"info\":\"FAIL\"}"));
            return false;
        }
    }

#if defined(ESP8266) || defined(ESP32)
    bool ConnectToNewWiFi(const char* ssid, const char* pass) {
        bool anyError = false;
        WiFi.persistent(true);      // ESP8266: saves credentials to flash. ESP32: harmless, ignored.
        if (!WiFi.begin(ssid, pass)) { anyError = true; }      // Connects to the AP.
        if (!WiFi.setAutoConnect(true)) {anyError = true; }    // ESP32: ensures reconnect on boot. ESP8266: also works.
        if (!WiFi.setAutoReconnect(true)) { anyError = true; } // ESP32: reconnect if connection drops. ESP8266: ignored (does nothing).
        WiFi.persistent(false); 
        return anyError == false;
    }
#endif

    HALOperationResult Exec_Scheduler_Cmd(ZeroCopyString& zcStr, CommandCallback cb) { // TODO to be implemented as a device
        std::string resStr;
        Scheduler::parseCmd(zcStr, resStr);

        const ZeroCopyString zcMsg = resStr.c_str();
        cb(zcMsg, CmdCbType::Control);
        return HALOperationResult::Success;
    }
    HALOperationResult Exec_Help_Cmd_All(ZeroCopyString& zcStr, CommandCallback cb) {

        BlockStreamer bs(cb, "help", BlockStreamer::DataType::Json);
        StringBuilderStreamer& sbs = bs.writer();

        sbs.write_json_object_begin();

        sbs.write_jsonMemberStart(F("root"));
        PrintHelpNodeJson(RootItem, sbs);

        sbs.write_json_object_end();

        return HALOperationResult::Success; 
    }
    HALOperationResult Exec_Help_Cmd_Type(ZeroCopyString& zcStr, CommandCallback cb) {
        return HALOperationResult::TODO_UnsupportedCommand;
    }
    HALOperationResult Exec_Help_Cmd_DeviceInstance(ZeroCopyString& zcStr, CommandCallback cb) {
        return HALOperationResult::TODO_UnsupportedCommand;
    }
    HALOperationResult Exec_Help_Cmd_By_Path(ZeroCopyString& zcStr, CommandCallback cb) {
        return HALOperationResult::TODO_UnsupportedCommand;
    }
    HALOperationResult Exec_System_Info(ZeroCopyString& zcStr, CommandCallback cb) {
#if defined(ESP8266) || defined(ESP32)
        std::string infoMsg = Info::getESP_info();
#else
        std::string infoMsg = "{\"mcu\":\"windows\"}";
#endif
        cb(infoMsg.c_str(), CmdCbType::Control);
        return HALOperationResult::Success;
    }
#if defined(ESP8266) || defined(ESP32)
    HALOperationResult Exec_System_Heap(ZeroCopyString& zcStr, CommandCallback cb) {
        BlockStreamer bs(cb, "heap", BlockStreamer::DataType::Json);
        StringBuilderStreamer& sbs = bs.writer();
        sbs.write_json_object_begin();
        sbs.write_jsonNumber(F("Heap"), ESP.getFreeHeap());
#if defined(ESP8266)
        sbs.write_json_value_separator(); sbs.write_jsonNumber(F("Max block"), ESP.getMaxFreeBlockSize());
#elif defined(ESP32)
        sbs.write_json_value_separator(); sbs.write_jsonNumber(F("Max block"), ESP.getMaxAllocHeap());
#endif
        sbs.write_json_object_end();
        return HALOperationResult::Success;
    }
    HALOperationResult Exec_System_Reset_Restart(ZeroCopyString& zcStr, CommandCallback cb) {
        BlockStreamer bs(cb, "reset", BlockStreamer::DataType::PlainText);
        StringBuilderStreamer& sbs = bs.writer();
        sbs.write(F("the system will now restart"));
        sbs.flush();
        
        ESP.restart();
        return HALOperationResult::Success;
    }
#endif
    HALOperationResult Exec_System_HeartbeatLed(ZeroCopyString& zcStr, CommandCallback cb) {
        BlockStreamer bs(cb, "HeartbeatLed", BlockStreamer::DataType::PlainText);
        StringBuilderStreamer& sbs = bs.writer();
        HeartbeatLed::parseCmd(zcStr, sbs);
        return HALOperationResult::Success;
    }
    HALOperationResult Exec_System_Build_Ver_Print(ZeroCopyString& zcStr, CommandCallback cb) {
        BlockStreamer bs(cb, "ver", BlockStreamer::DataType::Json);
        StringBuilderStreamer& sbs = bs.writer();
        sbs.write(F("{\"build_version\":\"" BUILD_VER_STR "\"}"));
        return HALOperationResult::Success;
    }
#if defined(ESP8266) || defined(ESP32)
    HALOperationResult Exec_WiFi_Scan(ZeroCopyString& zcStr, CommandCallback cb) {
        BlockStreamer bs(cb, "wifi/scan", BlockStreamer::DataType::Json);
        StringBuilderStreamer& sbs = bs.writer();
        sbs.write_json_array_begin();
        int n = WiFi.scanNetworks(false, true);
        for (int i = 0; i < n; i++) {
            if (i > 0) {
                sbs.write_json_value_separator();
            }
            String ssidB64 = b64urlEncode(WiFi.SSID(i).c_str());
            const char* enc;
            switch(WiFi.encryptionType(i)) {
#if defined(ESP32)
                case WIFI_AUTH_OPEN: enc = "OPEN"; break;
                case WIFI_AUTH_WEP: enc = "WEP"; break;
                case WIFI_AUTH_WPA_PSK: enc = "WPA"; break;
                case WIFI_AUTH_WPA2_PSK: enc = "WPA2"; break;
                case WIFI_AUTH_WPA_WPA2_PSK: enc = "WPA/WPA2"; break;
                case WIFI_AUTH_WPA2_ENTERPRISE: enc = "WPA2-E"; break;
#elif defined(ESP8266)
                case AUTH_OPEN: enc = "OPEN"; break;
                case AUTH_WEP: enc = "WEP"; break;
                case AUTH_WPA_PSK: enc = "WPA"; break;
                case AUTH_WPA2_PSK: enc = "WPA2"; break;
                case AUTH_WPA_WPA2_PSK: enc = "WPA/WPA2"; break;
                //case AUTH_WPA2_ENTERPRISE: enc = "WPA2-E"; break; // dont exist on esp8266

#endif
                default: enc = "UNK"; break;
            }
            sbs.write_json_object_begin();
            sbs.write_jsonString(F("ssid"), ssidB64.c_str());
            sbs.write_json_value_separator();
            sbs.write_jsonNumber(F("ch"), WiFi.channel(i));
            sbs.write_json_value_separator();
            sbs.write_jsonNumber(F("freq"), WiFi.channel(i) <= 14 ? 2400 : 5000); // crude 2.4/5 GHz
            sbs.write_json_value_separator();
            sbs.write_jsonNumber(F("rssi"), WiFi.RSSI(i));
            sbs.write_json_value_separator();
            sbs.write_jsonString(F("encryption"), enc);
            sbs.write_json_object_end();
        }
        sbs.write_json_array_end();
        return HALOperationResult::Success;
    }
    HALOperationResult Exec_WiFi_Set_b64(ZeroCopyString& zcStr, CommandCallback cb) {
        if (zcStr.CountChar(':') == 0) {
            cb(String(F("wifi/set_b64/error/missingparams\r\n")).c_str(), CmdCbType::Control);
            return HALOperationResult::ExecutionFailed;
        }
        DALHAL::ZeroCopyString zcSSID = zcStr.SplitOffHead(':');
        char ssid[33] = {0};
        char pass[65] = {0};

        int ssidLen = b64urlDecode((uint8_t*)ssid, sizeof(ssid) - 1, zcSSID);
        int passLen = b64urlDecode((uint8_t*)pass, sizeof(pass) - 1, zcStr);
        if (ssidLen <= 0 || passLen < 0) {
            if (ssidLen == 0) {
                cb(String(F("wifi/set_b64/error/ssid/empty\r\n")).c_str(), CmdCbType::Control);
            } else if (ssidLen == -1) {
                cb(String(F("wifi/set_b64/error/ssid/invalidchar\r\n")).c_str(), CmdCbType::Control);
            } else if (ssidLen == -2) {
                cb(String(F("wifi/set_b64/error/ssid/long\r\n")).c_str(), CmdCbType::Control);
            }

            if (passLen == -1) {
                cb(String(F("wifi/set_b64/error/pass/invalidchar\r\n")).c_str(), CmdCbType::Control);
            } else if (passLen == -2) {
                cb(String(F("wifi/set_b64/error/pass/long\r\n")).c_str(), CmdCbType::Control);
            }
            return HALOperationResult::ExecutionFailed;
        }
        if (!ConnectToNewWiFi(ssid, pass)) {
            cb(String(F("wifi/set_b64/FAIL\r\n")).c_str(), CmdCbType::Control);
            return HALOperationResult::ExecutionFailed;
        }
        cb(String(F("wifi/set_b64/OK\r\n")).c_str(), CmdCbType::Control);
        return HALOperationResult::Success;
    }
    HALOperationResult Exec_WiFi_Set_Json(ZeroCopyString& zcStr, CommandCallback cb) {
        DynamicJsonDocument doc(256);
        DeserializationError err = deserializeJson(doc, zcStr.start); // safe to use start here as that string is null terminated
        if(err) {
            cb(String(F("wifi/set_json/error/invalidjson\r\n")).c_str(), CmdCbType::Control);
            return HALOperationResult::ExecutionFailed;
        }

        const char* ssid = doc["ssid"];
        const char* pass = doc["pass"];

        if(ssid == nullptr || ssid[0] == '\0') {
            cb(String(F("wifi/set_json/error/ssid/empty\r\n")).c_str(), CmdCbType::Control);
            return HALOperationResult::ExecutionFailed;
        }

        if(pass == nullptr) pass = ""; // allow empty password if desired

        if (!ConnectToNewWiFi(ssid, pass)) {
            cb(String(F("wifi/set_json/FAIL\r\n")).c_str(), CmdCbType::Control);
            return HALOperationResult::ExecutionFailed;
        }
        cb(String(F("wifi/set_json/OK\r\n")).c_str(), CmdCbType::Control);
        return HALOperationResult::Success;
    }
#endif

    HALOperationResult Exec_Hal_Exec(ZeroCopyString& zcStr, CommandCallback cb) {
        CommandExecutor::ExecCmdParameters params(zcStr);
        // check if have uid given
        if (params.zcUid.IsEmpty()) {
            GlobalLogger.Error(F("uid path empty"));
            return HALOperationResult::InvalidArgument;
        }
        // first check if device exists
        UIDPath uidPath(params.zcUid);
        Device* outDevice = nullptr;
        DeviceFindResult devFindRes = DeviceManager::findDevice(uidPath, outDevice);
        if (devFindRes != DeviceFindResult::Success) {
            GlobalLogger.Error(F("device not found: "), params.zcUid);
            GlobalLogger.setLastEntrySource(DeviceFindResultToString(devFindRes));
            return HALOperationResult::DeviceNotFound;
        }

        HALOperationResult res = HALOperationResult::NotSet;

        auto fnRes = GetDeviceFunction<FunctionTypes::Exec>(outDevice, params.zcCmd);
        if (fnRes.result == HALOperationResult::Success) {
            res = fnRes.fn(outDevice);
        }

        if (res != HALOperationResult::Success) {
            String str = HALOperationResultToString(res);
            str += outDevice->Type;
            GlobalLogger.Error(F("HALOperationResult: "), str.c_str());
            return res;
        }
        BlockStreamer bs(cb, "hal/exec", BlockStreamer::DataType::Json);
        StringBuilderStreamer& sbs = bs.writer();
        sbs.write_json_object_begin();
        sbs.write_jsonString(F("info"), F("OK"));
        sbs.write_json_object_end();
        return HALOperationResult::Success;
    }
    
    HALOperationResult Exec_Hal_Read_String(ZeroCopyString& zcStr, CommandCallback cb) {
        BlockStreamer bs(cb, "hal/read/string", BlockStreamer::DataType::Json);
        Device* device = nullptr;
        CommandExecutor::ReadWriteCmdParameters params(zcStr);
        HALOperationResult opres = ValidateDeviceExistence(params, bs, &device);
        if (opres != HALOperationResult::Success) {
            return opres;
        }
        StringBuilderStreamer& sbs = bs.writer();
        sbs.write_json_object_begin();

        if (params.isHelpRequest == false) {
            auto fnRes = GetDeviceFunctionEntry<FunctionTypes::ReadString>(device, params.zcCmd);
            opres = fnRes.result;
            if (opres == HALOperationResult::Success) {
                opres = fnRes.entry->fn(device, params.zcParameters, sbs);
            }
        } else {
            sbs.write_jsonMemberStart(F("help"));
            if (params.cmdIsPresent) {
                opres = GetDeviceFunctions<FunctionTypes::ReadString>(device, sbs);
            } else {
                auto fnRes = GetDeviceFunctionEntry<FunctionTypes::ReadString>(device, params.zcCmd);
                opres = fnRes.result;
                if (opres == HALOperationResult::Success) {
                    fnRes.entry->help(sbs);
                }
            }
            if (opres != HALOperationResult::Success) {
                sbs.write_json_null(); // here we decide what to write on error
            }
        }
        sbs.write_json_object_end();
        return opres;
    }
    HALOperationResult Exec_Hal_Write_String(ZeroCopyString& zcStr, CommandCallback cb) {
        BlockStreamer bs(cb, "hal/write/string", BlockStreamer::DataType::Json);
        Device* device = nullptr;
        CommandExecutor::ReadWriteCmdParameters params(zcStr);
        HALOperationResult opres = ValidateDeviceExistence(params, bs, &device);
        if (opres != HALOperationResult::Success) {
            return opres;
        }
        StringBuilderStreamer& sbs = bs.writer();
        sbs.write_json_object_begin();

        if (params.isHelpRequest == false) {
            auto fnRes = GetDeviceFunctionEntry<FunctionTypes::WriteString>(device, params.zcCmd);
            opres = fnRes.result;
            if (opres == HALOperationResult::Success) {
                opres = fnRes.entry->fn(device, params.zcParameters, sbs);
            }
        } else {
            sbs.write_jsonMemberStart(F("help"));
            if (params.cmdIsPresent) {
                opres = GetDeviceFunctions<FunctionTypes::WriteString>(device, sbs);
            } else {
                auto fnRes = GetDeviceFunctionEntry<FunctionTypes::WriteString>(device, params.zcCmd);
                opres = fnRes.result;
                if (opres == HALOperationResult::Success) {
                    fnRes.entry->help(sbs);
                }
            }
            if (opres != HALOperationResult::Success) {
                sbs.write_json_null(); // here we decide what to write on error
            }
        }
        sbs.write_json_object_end();
        return opres;
    }

    HALOperationResult Exec_Hal_Read_Value(ZeroCopyString& zcStr, CommandCallback cb) {
        BlockStreamer bs(cb, "hal/read/value", BlockStreamer::DataType::Json);
        Device* device = nullptr;
        CommandExecutor::ReadWriteCmdParameters params(zcStr);
        HALOperationResult opres = ValidateDeviceExistence(params, bs, &device);
        if (opres != HALOperationResult::Success) {
            return opres;
        }
        StringBuilderStreamer& sbs = bs.writer();
        sbs.write_json_object_begin();

        if (params.isHelpRequest == false) {
            HALValue val;
            if (params.isBracketOp == false) {
                auto fnRes = GetDeviceFunction<FunctionTypes::ReadToHALValue>(device, params.zcCmd);
                opres = fnRes.result;
                if (opres == HALOperationResult::Success) {
                    opres = fnRes.fn(device, val);
                }
            } else {
                opres = HALOperationResult::TODO_UnsupportedCommand;
            }
            if (opres == HALOperationResult::Success) {
                sbs.write_jsonNumber(F("value"), val);
            }
        } else {
            sbs.write_jsonMemberStart(F("help"));
            if (params.isBracketOp == false) {
                if (params.cmdIsPresent) {
                    opres = GetDeviceFunctions<FunctionTypes::ReadToHALValue>(device, sbs);
                } else {
                    auto fnRes = GetDeviceFunctionEntry<FunctionTypes::ReadToHALValue>(device, params.zcCmd);
                    opres = fnRes.result;
                    if (opres == HALOperationResult::Success) {
                        fnRes.entry->help(sbs);
                    }
                }
            } else {
                opres = HALOperationResult::TODO_UnsupportedCommand;
            }
            if (opres != HALOperationResult::Success) {
                sbs.write_json_null(); // here we decide what to write on error
            }
        }

        sbs.write_json_object_end();
        return opres;
    }
    HALOperationResult Exec_Hal_Write_Value(ZeroCopyString& zcStr, CommandCallback cb) {
        BlockStreamer bs(cb, "hal/write/value", BlockStreamer::DataType::Json);
        Device* device = nullptr;
        CommandExecutor::ReadWriteCmdParameters params(zcStr);
        HALOperationResult opres = ValidateDeviceExistence(params, bs, &device);
        if (opres != HALOperationResult::Success) {
            return opres;
        }
        StringBuilderStreamer& sbs = bs.writer();
        sbs.write_json_object_begin();

        if (params.isHelpRequest == false) {
            HALValue halValue;
            NumberResult numRes = params.zcParameters.ConvertStringToNumber();
            if (numRes.type != NumberType::INVALID) {

                if (numRes.type == NumberType::FLOAT) {
                    halValue = numRes.f32;
                } else if (numRes.type == NumberType::UINT32) {
                    halValue = numRes.u32;
                } else if (numRes.type == NumberType::INT32) {
                    halValue = numRes.i32;
                }

            } else {
                bool bValue = 0;
                if (params.zcParameters.ConvertTo_bool(bValue) == false) {
                    sbs.write_jsonString(F("error"), F("Invalid value."));
                    sbs.write_json_object_end();
                    return HALOperationResult::InvalidArgument;
                }
                halValue = bValue;
            }
            if (params.isBracketOp == false) {
                auto fnRes = GetDeviceFunction<FunctionTypes::WriteHALValue>(device, params.zcCmd);
                opres = fnRes.result;
                if (opres == HALOperationResult::Success) {
                    opres = fnRes.fn(device, halValue);
                }
            } else {
                opres = HALOperationResult::TODO_UnsupportedCommand;
            }
            if (opres == HALOperationResult::Success) {
                sbs.write_jsonString(F("info"), F("Value written"));
                sbs.write_json_value_separator();
                sbs.write_jsonNumber(F("value"), halValue);
                sbs.write_json_object_end();
            }
        }
        else {
            sbs.write_jsonMemberStart(F("help"));
            if (params.isBracketOp == false) {
                if (params.cmdIsPresent) {
                    opres = GetDeviceFunctions<FunctionTypes::WriteHALValue>(device, sbs);
                } else {
                    auto fnRes = GetDeviceFunctionEntry<FunctionTypes::WriteHALValue>(device, params.zcCmd);
                    opres = fnRes.result;
                    if (opres == HALOperationResult::Success) {
                        fnRes.entry->help(sbs);
                    }
                }
            } else {
                opres = HALOperationResult::TODO_UnsupportedCommand;
            }
            if (opres != HALOperationResult::Success) {
                sbs.write_json_null(); // here we decide what to write on error
            }
        }
        
        sbs.write_json_object_end();
        return opres;
    }

    HALOperationResult Exec_Hal_Config_Reload(ZeroCopyString& zcStr, CommandCallback cb) {
        bool anyErrors = reloadJSON(zcStr, cb) == false;

        if (anyErrors) { return HALOperationResult::ExecutionFailed; }

        anyErrors = (ScriptEngine::ValidateAndLoadAllActiveScripts() == false);

        if (anyErrors) { return HALOperationResult::ExecutionFailed; }
        
        return HALOperationResult::Success;
        
    }
    HALOperationResult Exec_Hal_Config_Unload(ZeroCopyString& zcStr, CommandCallback cb) {

        return HALOperationResult::TODO_UnsupportedCommand;
    }

    HALOperationResult Exec_Hal_Scripts_Reload(ZeroCopyString& zcStr, CommandCallback cb) {
        BlockStreamer bs(cb, "hal/scripts/reload", BlockStreamer::DataType::Json);
        if (ScriptEngine::ValidateAndLoadAllActiveScripts()) {
            bs.writer().write_jsonString(F("info"), F("ok"));
            return HALOperationResult::Success;
        }
        bs.writer().write_jsonString(F("error"), F("could not ValidateAndLoadAllActiveScripts"));
        return HALOperationResult::ExecutionFailed;
    }

    HALOperationResult Exec_Hal_Scripts_Unload(ZeroCopyString& zcStr, CommandCallback cb) {
        return HALOperationResult::TODO_UnsupportedCommand;
    }
    
    HALOperationResult Exec_Hal_Scripts_Stop(ZeroCopyString& zcStr, CommandCallback cb) {
        BlockStreamer bs(cb, "hal/scripts/stop", BlockStreamer::DataType::Json);
        ScriptEngine::ScriptBlocks::running = false;
        bs.writer().write_jsonString(F("info"), F("stopped"));
        return HALOperationResult::Success;
    }
    HALOperationResult Exec_Hal_Scripts_Start(ZeroCopyString& zcStr, CommandCallback cb) {
        BlockStreamer bs(cb, "hal/scripts/start", BlockStreamer::DataType::Json);
        ScriptEngine::ScriptBlocks::running = true;
        bs.writer().write_jsonString(F("info"), F("started"));
        return HALOperationResult::Success;
    }

    HALOperationResult Exec_Hal_GetAvailableGPIOs(ZeroCopyString& zcStr, CommandCallback cb) {
        BlockStreamer bs(cb, "getAvailableGPIOs", BlockStreamer::DataType::Json);
        GPIO_manager::GetList(zcStr, bs.writer());
        return HALOperationResult::Success;
    }
    HALOperationResult Exec_Hal_PrintDevices(ZeroCopyString& zcStr, CommandCallback cb) {
        BlockStreamer bs(cb, "printDevices", BlockStreamer::DataType::Json);
        DeviceManager::PrintTo(bs.writer());
        return HALOperationResult::Success;
    }
    HALOperationResult Exec_PrintLog(ZeroCopyString& zcStr, CommandCallback cb) {
        BlockStreamer bs(cb, "logs", BlockStreamer::DataType::PlainText);
        GlobalLogger.printAllLogs(bs.writer());
        return HALOperationResult::Success;
    }
    HALOperationResult Exec_Hal_PrintRegistry_Types(ZeroCopyString& zcStr, CommandCallback cb) {
        DALHAL::BlockStreamer bs(cb, "registry", DALHAL::BlockStreamer::DataType::Json);
        Registry::PrintTo(RootDevicesRegistry, Registry::PrintMode::Types, bs.writer());
        return HALOperationResult::Success;
    }
    HALOperationResult Exec_Hal_PrintRegistry_Functions(ZeroCopyString& zcStr, CommandCallback cb) {
        DALHAL::BlockStreamer bs(cb, "registry", DALHAL::BlockStreamer::DataType::Json);
        Registry::PrintTo(RootDevicesRegistry, Registry::PrintMode::Functions, bs.writer());
        return HALOperationResult::Success;
    }
    HALOperationResult Exec_Hal_PrintRegistry_Events(ZeroCopyString& zcStr, CommandCallback cb) {
        DALHAL::BlockStreamer bs(cb, "registry", DALHAL::BlockStreamer::DataType::Json);
        Registry::PrintTo(RootDevicesRegistry, Registry::PrintMode::Events, bs.writer());
        return HALOperationResult::Success;
    }
    HALOperationResult Exec_Hal_PrintJsonSchemas(ZeroCopyString& zcStr, CommandCallback cb) {
        DALHAL::BlockStreamer bs(cb, "schema", BlockStreamer::DataType::Json);
        JsonSchema::ToJsonString::buildCompleteJsonSchemasStartingFrom(RootDevicesRegistry, bs.writer());
        return HALOperationResult::Success;
    }
}