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
        zcUid = zcStr.SplitOffHead('/');
        zcCmd = zcUid.SplitOffTail('#');
        zcParameters = zcStr; // the value is the rest
    }

    CommandExecutor::ExecCmdParameters::ExecCmdParameters(ZeroCopyString& zcStr) {
        zcUid = zcStr.SplitOffHead('/');
        zcCmd = zcUid.SplitOffTail('#');
    }

#define DALHAL_CMD_CHILDREN(n) n, DALHAL_ARRAY_COUNT(n)

    void SetWifiCredentialsAndRestart(const char* ssid, const char* pass);
    bool reloadJSON(ZeroCopyString& zcStr, CommandCallback cb);

    HALOperationResult Exec_Scheduler_Cmd(ZeroCopyString& zcStr, CommandCallback cb); // TODO to be implemented as a device

    HALOperationResult Exec_Help_Cmd(ZeroCopyString& zcStr, CommandCallback cb);
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
    HALOperationResult Exec_Hal_Read_UInt32(ZeroCopyString& zcStr, CommandCallback cb);
    HALOperationResult Exec_Hal_Read_Int32(ZeroCopyString& zcStr, CommandCallback cb);
    HALOperationResult Exec_Hal_Read_Bool(ZeroCopyString& zcStr, CommandCallback cb);
    HALOperationResult Exec_Hal_Read_Float(ZeroCopyString& zcStr, CommandCallback cb);
    HALOperationResult Exec_Hal_Write_String(ZeroCopyString& zcStr, CommandCallback cb);
    HALOperationResult Exec_Hal_Write_UInt32(ZeroCopyString& zcStr, CommandCallback cb);
    HALOperationResult Exec_Hal_Write_Int32(ZeroCopyString& zcStr, CommandCallback cb);
    HALOperationResult Exec_Hal_Write_Bool(ZeroCopyString& zcStr, CommandCallback cb);
    HALOperationResult Exec_Hal_Write_Float(ZeroCopyString& zcStr, CommandCallback cb);

    HALOperationResult Exec_Hal_Config_Reload(ZeroCopyString& zcStr, CommandCallback cb);
    HALOperationResult Exec_Hal_Config_Unload(ZeroCopyString& zcStr, CommandCallback cb);

    HALOperationResult Exec_Hal_Scripts_Reload(ZeroCopyString& zcStr, CommandCallback cb);
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
        { CE_MATCH_EMIT_STR("string"), Exec_Hal_Write_String, CE_EMIT_STR("hal write string") },
        { CE_MATCH_EMIT_STR("uint32"), Exec_Hal_Write_UInt32, CE_EMIT_STR("hal write uint32") },
        { CE_MATCH_EMIT_STR("int32"), Exec_Hal_Write_Int32, CE_EMIT_STR("hal write int32") },
        { CE_MATCH_EMIT_STR("bool"), Exec_Hal_Write_Bool, CE_EMIT_STR("hal write bool") },
        { CE_MATCH_EMIT_STR("float"), Exec_Hal_Write_Float, CE_EMIT_STR("hal write float") },
    };

    static constexpr CommandNode HalReadItems[] = {
        { CE_MATCH_EMIT_STR("string"), Exec_Hal_Read_String, CE_EMIT_STR("hal read string") },
        { CE_MATCH_EMIT_STR("uint32"), Exec_Hal_Read_UInt32, CE_EMIT_STR("hal read uint32") },
        { CE_MATCH_EMIT_STR("int32"), Exec_Hal_Read_Int32, CE_EMIT_STR("hal read int32") },
        { CE_MATCH_EMIT_STR("bool"), Exec_Hal_Read_Bool, CE_EMIT_STR("hal read bool") },
        { CE_MATCH_EMIT_STR("float"), Exec_Hal_Read_Float, CE_EMIT_STR("hal read float") },
    };

    static constexpr CommandNode HalScriptItems[] = {
        { CE_MATCH_EMIT_STR("reload"), Exec_Hal_Scripts_Reload, CE_EMIT_STR("scripts reload") },
        { CE_MATCH_EMIT_STR("stop"), Exec_Hal_Scripts_Stop, CE_EMIT_STR("scripts stop/pause execution") },
        { CE_MATCH_EMIT_STR("start"), Exec_Hal_Scripts_Start, CE_EMIT_STR("scripts start/resume execution") },
    };

    static constexpr CommandNode HalConfigItems[] = {
        { CE_MATCH_EMIT_STR("reload"), Exec_Hal_Config_Reload, CE_EMIT_STR("hal config first validate cfg from file and if success, 1. unloads the current cfg. 2. load new cfg. 3. reload/validate script.") },
        { CE_MATCH_EMIT_STR("unload"), Exec_Hal_Config_Unload, CE_EMIT_STR("hal config/script unload, this makes the current cfg clean, can be used to make sure that the memory is clean before loading new config") },
    };

    static constexpr CommandNode HalMetaRegistryItems[] = {
        { CE_MATCH_EMIT_STR("types"), Exec_Hal_PrintRegistry_Types, CE_EMIT_STR("print device type registry basedata") },
        { CE_MATCH_EMIT_STR("functions"), Exec_Hal_PrintRegistry_Functions, CE_EMIT_STR("print device type registry with functions") },
        { CE_MATCH_EMIT_STR("events"), Exec_Hal_PrintRegistry_Events, CE_EMIT_STR("print device type registry with events") },
        { CE_MATCH_EMIT_STR("cfgschema"), Exec_Hal_PrintJsonSchemas, CE_EMIT_STR("print config json schema") }
    };

    static constexpr CommandNode HalMetaItems[] = {
        { CE_MATCH_EMIT_STR("gpio"), Exec_Hal_GetAvailableGPIOs, CE_EMIT_STR("get a list of available GPIO on this target and their functions") },
        { CE_MATCH_EMIT_STR("devices"), Exec_Hal_PrintDevices, CE_EMIT_STR("print all current loaded devices") },
        { CE_MATCH_EMIT_STR("reg"), DALHAL_CMD_CHILDREN(HalMetaRegistryItems), CE_EMIT_STR("print registry metadata")}
    };

    static constexpr CommandNode HalItems[] = {
        { CE_MATCH_EMIT_STR("exec"), Exec_Hal_Exec, CE_EMIT_STR("run device exec cmd") },
        { CE_MATCH_EMIT_STR("write"), DALHAL_CMD_CHILDREN(HalWriteItems), CE_EMIT_STR("run device write cmds") },
        { CE_MATCH_EMIT_STR("read"), DALHAL_CMD_CHILDREN(HalReadItems), CE_EMIT_STR("run device read cmds") },
        { CE_MATCH_EMIT_STR("config"), DALHAL_CMD_CHILDREN(HalConfigItems), CE_EMIT_STR("hal config cmds") },
        { CE_MATCH_EMIT_STR("scripts"), DALHAL_CMD_CHILDREN(HalScriptItems), CE_EMIT_STR("script specific commands") },
        { CE_MATCH_EMIT_STR("meta"), DALHAL_CMD_CHILDREN(HalMetaItems), CE_EMIT_STR("print metadata")}
    };
#if defined(ESP8266) || defined(ESP32)
    static constexpr CommandNode WiFiItems[] = {
        { CE_MATCH_EMIT_STR("scan"), Exec_WiFi_Scan, CE_EMIT_STR("Scan WiFi for AP:s") },
        { CE_MATCH_EMIT_STR("set_b64"), Exec_WiFi_Set_b64, CE_EMIT_STR("Set current WiFi settings from the given b64 encoded string") },
        { CE_MATCH_EMIT_STR("set_json"), Exec_WiFi_Set_Json, CE_EMIT_STR("Set current WiFi settings from the given json encoded string") },
    };
#endif
    static constexpr CommandNode SystemItems[] = {
        
        { CE_MATCH_EMIT_STR("info"), Exec_System_Info, CE_EMIT_STR("System info") },
#if defined(ESP8266) || defined(ESP32)
        { CE_MATCH_EMIT_STR("heap"), Exec_System_Heap, CE_EMIT_STR("Print heap info") },
        { CE_MATCH_EMIT_STR("reset"), Exec_System_Reset_Restart, CE_EMIT_STR("Reset the system") },
        { CE_MATCH_EMIT_STR("restart"), Exec_System_Reset_Restart, CE_EMIT_STR("Restart the system (same as reset)") },
#endif
        { CE_MATCH_EMIT_STR("HeartbeatLed"), Exec_System_HeartbeatLed, CE_EMIT_STR("HeartBeatLed set timings") },
        { CE_MATCH_EMIT_STR("ver"), Exec_System_Build_Ver_Print, CE_EMIT_STR("Print current build version") },
    };

    static constexpr CommandNode RootItems[] = {
        { CE_MATCH_EMIT_STR("hal"), DALHAL_CMD_CHILDREN(HalItems), CE_EMIT_STR("HAL subsystem") },
        { CE_MATCH_EMIT_STR("system"), DALHAL_CMD_CHILDREN(SystemItems), CE_EMIT_STR("System commands") },
#if defined(ESP8266) || defined(ESP32)
        { CE_MATCH_EMIT_STR("wifi"), DALHAL_CMD_CHILDREN(WiFiItems), CE_EMIT_STR("WiFi management") },
#endif
        { CE_MATCH_EMIT_STR("printLog"), Exec_PrintLog, CE_EMIT_STR("print log") },
        { CE_MATCH_EMIT_STR("schedule"), Exec_Scheduler_Cmd, CE_EMIT_STR("Schedule commands") }, // to be removed in the future as it would be implemented as a device
        { CE_MATCH_EMIT_STR("help"), Exec_Help_Cmd, CE_EMIT_STR("Show help") }
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

        sbs.write_json_value_separator();

        // help
        sbs.write_jsonMemberStart(F("help"));
        node.help(sbs);

        // children
        sbs.write_json_value_separator();
        sbs.write_jsonMemberStart(F("children"));
        sbs.write_json_array_begin();

        for (size_t i = 0; i < node.children_count; i++) {
            if (i > 0) {
                sbs.write_json_value_separator();
            }

            PrintHelpNodeJson(node.children[i], sbs);
        }

        sbs.write_json_array_end();

        sbs.write_json_object_end();
    }

    HALOperationResult PrecheckHalReadWriteExecOperation(CommandExecutor::ReadWriteCmdParameters& params, BlockStreamer& bs, Device** outDevice) {
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
            sbs.write('"');
            sbs.write(F("device not found: "));
            sbs.write(params.zcUid);
            sbs.write('"');
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
    void SetWifiCredentialsAndRestart(const char* ssid, const char* pass) {
        WiFi.persistent(true);      // ESP8266: saves credentials to flash. ESP32: harmless, ignored.
        WiFi.begin(ssid, pass);     // Connects to the AP.
        WiFi.setAutoConnect(true);  // ESP32: ensures reconnect on boot. ESP8266: also works.
        WiFi.setAutoReconnect(true);// ESP32: reconnect if connection drops. ESP8266: ignored (does nothing).
        
        WiFi.disconnect(false, true); // ensures save on ESP32
        delay(200); 
        
        // Restart the device so it boots with the saved credentials
        ESP.restart();
    }
#endif

    HALOperationResult Exec_Scheduler_Cmd(ZeroCopyString& zcStr, CommandCallback cb) { // TODO to be implemented as a device
        std::string resStr;
        Scheduler::parseCmd(zcStr, resStr);

        const ZeroCopyString zcMsg = resStr.c_str();
        cb(zcMsg, CmdCbType::Control);
        return HALOperationResult::Success;
    }
    HALOperationResult Exec_Help_Cmd(ZeroCopyString& zcStr, CommandCallback cb) {

        BlockStreamer bs(cb, "help", BlockStreamer::DataType::Json);
        StringBuilderStreamer& sbs = bs.writer();

        sbs.write_json_object_begin();

        sbs.write_jsonMemberStart(F("root"));
        PrintHelpNodeJson(RootItem, sbs);

        sbs.write_json_object_end();

        return HALOperationResult::Success; 
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

        cb(String(F("wifi/set_b64/OK\r\n")).c_str(), CmdCbType::Control);
        delay(50);
        
        SetWifiCredentialsAndRestart(ssid, pass);
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

        cb(String(F("wifi/set_json/OK\r\n")).c_str(), CmdCbType::Control);

        SetWifiCredentialsAndRestart(ssid, pass);
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
        return HALOperationResult::Success;
    }
    HALOperationResult Exec_Hal_Read_String(ZeroCopyString& zcStr, CommandCallback cb) {
        BlockStreamer bs(cb, "hal/read/string", BlockStreamer::DataType::Json);
        Device* device = nullptr;
        CommandExecutor::ReadWriteCmdParameters params(zcStr);
        HALOperationResult opres = PrecheckHalReadWriteExecOperation(params, bs, &device);
        if (opres != HALOperationResult::Success) {
            return opres;
        }
        StringBuilderStreamer& sbs = bs.writer();
        sbs.write_json_object_begin();

        auto fnRes = GetDeviceFunction<FunctionTypes::ReadString>(device, params.zcCmd);
        if (fnRes.result == HALOperationResult::Success) {

            opres = fnRes.fn(device, params.zcParameters, sbs);
            if (opres != HALOperationResult::Success) {
                sbs.write_jsonMemberStart(F("error"));
                sbs.write_jsonQuoted(HALOperationResultToString(opres));
            }
        } else {
            sbs.write_jsonMemberStart(F("error"));
            sbs.write_jsonQuoted(HALOperationResultToString(fnRes.result));
            opres = fnRes.result;
        }

        sbs.write_json_object_end();
        return opres;
    }
    HALOperationResult Exec_Hal_Read_UInt32(ZeroCopyString& zcStr, CommandCallback cb) {
        BlockStreamer bs(cb, "hal/read/uint32", BlockStreamer::DataType::Json);
        Device* device = nullptr;
        CommandExecutor::ReadWriteCmdParameters params(zcStr);
        HALOperationResult opres = PrecheckHalReadWriteExecOperation(params, bs, &device);
        if (opres != HALOperationResult::Success) {
            return opres;
        }
        StringBuilderStreamer& sbs = bs.writer();
        sbs.write_json_object_begin();
        HALValue val;
        auto fnRes = GetDeviceFunction<FunctionTypes::ReadToHALValue>(device, params.zcCmd);
        if (fnRes.result == HALOperationResult::Success) {
            
            opres = fnRes.fn(device, val);
            if (opres == HALOperationResult::Success) {
                sbs.write_jsonNumber(F("value"), val.toUInt());
            } else {
                sbs.write_jsonMemberStart(F("error"));
                sbs.write_jsonQuoted(HALOperationResultToString(opres));
            }
        } else {
            sbs.write_jsonMemberStart(F("error"));
            sbs.write_jsonQuoted(HALOperationResultToString(fnRes.result));
        }

        sbs.write_json_object_end();
        return opres;
    }
    HALOperationResult Exec_Hal_Read_Int32(ZeroCopyString& zcStr, CommandCallback cb) {
        BlockStreamer bs(cb, "hal/read/int32", BlockStreamer::DataType::Json);
        Device* device = nullptr;
        CommandExecutor::ReadWriteCmdParameters params(zcStr);
        HALOperationResult opres = PrecheckHalReadWriteExecOperation(params, bs, &device);
        if (opres != HALOperationResult::Success) {
            return opres;
        }
        StringBuilderStreamer& sbs = bs.writer();
        sbs.write_json_object_begin();
        HALValue val;
        auto fnRes = GetDeviceFunction<FunctionTypes::ReadToHALValue>(device, params.zcCmd);
        if (fnRes.result == HALOperationResult::Success) {
            
            opres = fnRes.fn(device, val);
            if (opres == HALOperationResult::Success) {
                sbs.write_jsonNumber(F("value"), val.toInt());
            } else {
                sbs.write_jsonMemberStart(F("error"));
                sbs.write_jsonQuoted(HALOperationResultToString(opres));
            }
        } else {
            sbs.write_jsonMemberStart(F("error"));
            sbs.write_jsonQuoted(HALOperationResultToString(fnRes.result));
        }

        sbs.write_json_object_end();
        return opres;
    }
    HALOperationResult Exec_Hal_Read_Bool(ZeroCopyString& zcStr, CommandCallback cb) {
        BlockStreamer bs(cb, "hal/read/bool", BlockStreamer::DataType::Json);
        Device* device = nullptr;
        CommandExecutor::ReadWriteCmdParameters params(zcStr);
        HALOperationResult opres = PrecheckHalReadWriteExecOperation(params, bs, &device);
        if (opres != HALOperationResult::Success) {
            return opres;
        }
        StringBuilderStreamer& sbs = bs.writer();
        sbs.write_json_object_begin();

        HALValue val;
        auto fnRes = GetDeviceFunction<FunctionTypes::ReadToHALValue>(device, params.zcCmd);
        if (fnRes.result == HALOperationResult::Success) {
            
            opres = fnRes.fn(device, val);
            if (opres == HALOperationResult::Success) {
                sbs.write_jsonNumber(F("value"), val.toBool());
            } else {
                sbs.write_jsonMemberStart(F("error"));
                sbs.write_jsonQuoted(HALOperationResultToString(opres));
            }
        } else {
            sbs.write_jsonMemberStart(F("error"));
            sbs.write_jsonQuoted(HALOperationResultToString(fnRes.result));
        }

        sbs.write_json_object_end();
        return opres;
    }
    HALOperationResult Exec_Hal_Read_Float(ZeroCopyString& zcStr, CommandCallback cb) {
        BlockStreamer bs(cb, "hal/read/float", BlockStreamer::DataType::Json);
        Device* device = nullptr;
        CommandExecutor::ReadWriteCmdParameters params(zcStr);
        HALOperationResult opres = PrecheckHalReadWriteExecOperation(params, bs, &device);
        if (opres != HALOperationResult::Success) {
            return opres;
        }
        StringBuilderStreamer& sbs = bs.writer();
        sbs.write_json_object_begin();

        HALValue val;
        auto fnRes = GetDeviceFunction<FunctionTypes::ReadToHALValue>(device, params.zcCmd);
        if (fnRes.result == HALOperationResult::Success) {
            
            opres = fnRes.fn(device, val);
            if (opres == HALOperationResult::Success) {
                sbs.write_jsonNumber(F("value"), val.toFloat());
            } else {
                sbs.write_jsonMemberStart(F("error"));
                sbs.write_jsonQuoted(HALOperationResultToString(opres));
            }
        } else {
            sbs.write_jsonMemberStart(F("error"));
            sbs.write_jsonQuoted(HALOperationResultToString(fnRes.result));
        }

        sbs.write_json_object_end();
        return opres;
    }
    HALOperationResult Exec_Hal_Write_String(ZeroCopyString& zcStr, CommandCallback cb) {
        BlockStreamer bs(cb, "hal/write/string", BlockStreamer::DataType::Json);
        Device* device = nullptr;
        CommandExecutor::ReadWriteCmdParameters params(zcStr);
        HALOperationResult opres = PrecheckHalReadWriteExecOperation(params, bs, &device);
        if (opres != HALOperationResult::Success) {
            return opres;
        }
        StringBuilderStreamer& sbs = bs.writer();
        sbs.write_json_object_begin();

        auto fnRes = GetDeviceFunction<FunctionTypes::WriteString>(device, params.zcCmd);
        if (fnRes.result == HALOperationResult::Success) {

            opres = fnRes.fn(device, params.zcParameters, sbs);
            if (opres == HALOperationResult::Success) {
                sbs.write_jsonString(F("info"), F("ok"));
            } else {
                sbs.write_jsonMemberStart(F("error"));
                sbs.write_jsonQuoted(HALOperationResultToString(opres));
            }
        } else {
            sbs.write_jsonMemberStart(F("error"));
            sbs.write_jsonQuoted(HALOperationResultToString(fnRes.result));
            opres = fnRes.result;
        }

        sbs.write_json_object_end();
        return opres;
    }
    HALOperationResult Exec_Hal_Write_UInt32(ZeroCopyString& zcStr, CommandCallback cb) {
        BlockStreamer bs(cb, "hal/write/uint32", BlockStreamer::DataType::Json);
        Device* device = nullptr;
        CommandExecutor::ReadWriteCmdParameters params(zcStr);
        HALOperationResult opres = PrecheckHalReadWriteExecOperation(params, bs, &device);
        if (opres != HALOperationResult::Success) {
            return opres;
        }
        StringBuilderStreamer& sbs = bs.writer();
        sbs.write_json_object_begin();
        // Convert value to integer
        uint32_t uintValue = 0;
        if (params.zcParameters.ConvertTo_uint32(uintValue) == false) {
            sbs.write_jsonString(F("error"), F("Invalid uint32 value."));
        } else {
            HALValue halValue = uintValue;

            auto fnRes = GetDeviceFunction<FunctionTypes::WriteHALValue>(device, params.zcCmd);
            if (fnRes.result == HALOperationResult::Success) {
                opres = fnRes.fn(device, halValue);
            } else {
                opres = fnRes.result;
            }
            
            if (opres == HALOperationResult::Success) {
                sbs.write_jsonString(F("info"), F("Value written"));
                sbs.write_json_value_separator();
                sbs.write_jsonNumber(F("value"), uintValue);
                sbs.write_json_object_end();
            } else {
                sbs.write_jsonMemberStart(F("error"));
                sbs.write_jsonQuoted(HALOperationResultToString(opres));
            }
        }

        sbs.write_json_object_end();
        return opres;
    }
    HALOperationResult Exec_Hal_Write_Int32(ZeroCopyString& zcStr, CommandCallback cb) {
        BlockStreamer bs(cb, "hal/write/int32", BlockStreamer::DataType::Json);
        Device* device = nullptr;
        CommandExecutor::ReadWriteCmdParameters params(zcStr);
        HALOperationResult opres = PrecheckHalReadWriteExecOperation(params, bs, &device);
        if (opres != HALOperationResult::Success) {
            return opres;
        }
        StringBuilderStreamer& sbs = bs.writer();
        sbs.write_json_object_begin();
        // Convert value to integer
        int32_t intValue = 0;
        if (params.zcParameters.ConvertTo_int32(intValue) == false) {
            sbs.write_jsonString(F("error"), F("Invalid int32 value."));
        } else {
            HALValue halValue = intValue;

            auto fnRes = GetDeviceFunction<FunctionTypes::WriteHALValue>(device, params.zcCmd);
            if (fnRes.result == HALOperationResult::Success) {
                opres = fnRes.fn(device, halValue);
            } else {
                opres = fnRes.result;
            }
            
            if (opres == HALOperationResult::Success) {
                sbs.write_jsonString(F("info"), F("Value written"));
                sbs.write_json_value_separator();
                sbs.write_jsonNumber(F("value"), intValue);
                sbs.write_json_object_end();
            } else {
                sbs.write_jsonMemberStart(F("error"));
                sbs.write_jsonQuoted(HALOperationResultToString(opres));
            }
        }

        sbs.write_json_object_end();
        return opres;
    }
    HALOperationResult Exec_Hal_Write_Bool(ZeroCopyString& zcStr, CommandCallback cb) {
        BlockStreamer bs(cb, "hal/write/bool", BlockStreamer::DataType::Json);
        Device* device = nullptr;
        CommandExecutor::ReadWriteCmdParameters params(zcStr);
        HALOperationResult opres = PrecheckHalReadWriteExecOperation(params, bs, &device);
        if (opres != HALOperationResult::Success) {
            return opres;
        }
        StringBuilderStreamer& sbs = bs.writer();
        sbs.write_json_object_begin();
        // Convert value to integer
        bool bValue = 0;
        if (params.zcParameters.ConvertTo_bool(bValue) == false) {
            sbs.write_jsonString(F("error"), F("Invalid bool value."));
        } else {
            HALValue halValue = bValue;

            auto fnRes = GetDeviceFunction<FunctionTypes::WriteHALValue>(device, params.zcCmd);
            if (fnRes.result == HALOperationResult::Success) {
                opres = fnRes.fn(device, halValue);
            } else {
                opres = fnRes.result;
            }
            
            if (opres == HALOperationResult::Success) {
                sbs.write_jsonString(F("info"), F("Value written"));
                sbs.write_json_value_separator();
                sbs.write_jsonBool(F("value"), bValue);
                sbs.write_json_object_end();
            } else {
                sbs.write_jsonMemberStart(F("error"));
                sbs.write_jsonQuoted(HALOperationResultToString(opres));
            }
        }

        sbs.write_json_object_end();
        return opres;
    }
    HALOperationResult Exec_Hal_Write_Float(ZeroCopyString& zcStr, CommandCallback cb) {
        BlockStreamer bs(cb, "hal/write/float", BlockStreamer::DataType::Json);
        Device* device = nullptr;
        CommandExecutor::ReadWriteCmdParameters params(zcStr);
        HALOperationResult opres = PrecheckHalReadWriteExecOperation(params, bs, &device);
        if (opres != HALOperationResult::Success) {
            return opres;
        }
        StringBuilderStreamer& sbs = bs.writer();
        sbs.write_json_object_begin();
        // Convert value to float
        float floatValue = 0;
        if (params.zcParameters.ConvertTo_float(floatValue) == false) {
            sbs.write_jsonString(F("error"), F("Invalid float value."));
        } else {
            HALValue halValue = floatValue;

            auto fnRes = GetDeviceFunction<FunctionTypes::WriteHALValue>(device, params.zcCmd);
            if (fnRes.result == HALOperationResult::Success) {
                opres = fnRes.fn(device, halValue);
            } else {
                opres = fnRes.result;
            }
            
            if (opres == HALOperationResult::Success) {
                sbs.write_jsonString(F("info"), F("Value written"));
                sbs.write_json_value_separator();
                sbs.write_jsonNumber(F("value"), floatValue);
                sbs.write_json_object_end();
            } else {
                sbs.write_jsonMemberStart(F("error"));
                sbs.write_jsonQuoted(HALOperationResultToString(opres));
            }
        }

        sbs.write_json_object_end();
        return opres;
        return HALOperationResult::Success;
    }

    HALOperationResult Exec_Hal_Config_Reload(ZeroCopyString& zcStr, CommandCallback cb) {
        bool anyErrors = reloadJSON(zcStr, cb) == false;

        if (anyErrors == false) {
            
            anyErrors = ScriptEngine::ValidateAndLoadAllActiveScripts() == false;
            return anyErrors ? HALOperationResult::Success : HALOperationResult::ExecutionFailed;
        }
        return HALOperationResult::ExecutionFailed;
    }
    HALOperationResult Exec_Hal_Config_Unload(ZeroCopyString& zcStr, CommandCallback cb) {

        return HALOperationResult::Success;
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
    HALOperationResult Exec_Hal_Scripts_Stop(ZeroCopyString& zcStr, CommandCallback cb) {
        BlockStreamer bs(cb, "hal/scripts/stop", BlockStreamer::DataType::Json);
        ScriptEngine::ScriptsBlock::running = false;
        bs.writer().write_jsonString(F("info"), F("stopped"));
        return HALOperationResult::Success;
    }
    HALOperationResult Exec_Hal_Scripts_Start(ZeroCopyString& zcStr, CommandCallback cb) {
        BlockStreamer bs(cb, "hal/scripts/start", BlockStreamer::DataType::Json);
        ScriptEngine::ScriptsBlock::running = true;
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