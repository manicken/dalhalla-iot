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
    HALOperationResult Exec_Hal_PrintLog(ZeroCopyString& zcStr, CommandCallback cb);
    HALOperationResult Exec_Hal_PrintRegistry(ZeroCopyString& zcStr, CommandCallback cb);
    HALOperationResult Exec_Hal_PrintJsonSchemas(ZeroCopyString& zcStr, CommandCallback cb);

    static constexpr CommandNode HalWriteItems[] = {
        { "string", Exec_Hal_Write_String, "hal write string" },
        { "uint32", Exec_Hal_Write_UInt32, "hal write uint32" },
        { "int32", Exec_Hal_Write_Int32, "hal write int32" },
        { "bool", Exec_Hal_Write_Bool, "hal write bool" },
        { "float", Exec_Hal_Write_Float, "hal write float" },
    };

    static constexpr CommandNode HalReadItems[] = {
        { "string", Exec_Hal_Read_String, "hal read string" },
        { "uint32", Exec_Hal_Read_UInt32, "hal read uint32" },
        { "int32", Exec_Hal_Read_Int32, "hal read int32" },
        { "bool", Exec_Hal_Read_Bool, "hal read bool" },
        { "float", Exec_Hal_Read_Float, "hal read float" },
    };

    static constexpr CommandNode HalScriptItems[] = {
        { "reload", Exec_Hal_Scripts_Reload, "scripts reload" },
        { "stop", Exec_Hal_Scripts_Stop, "scripts stop/pause execution" },
        { "start", Exec_Hal_Scripts_Start, "scripts start/resume execution" },
    };

    static constexpr CommandNode HalConfigItems[] = {
        { "reload", Exec_Hal_Config_Reload, "hal config first validate cfg from file and if success, 1. unloads the current cfg. 2. load new cfg. 3. reload/validate script." },
        { "unload", Exec_Hal_Config_Unload, "hal config/script unload, this makes the current cfg clean, can be used to make sure that the memory is clean before loading new config" },
    };

    static constexpr CommandNode HalItems[] = {
        { "exec", Exec_Hal_Exec, "run device exec cmd" },
        { "write", DALHAL_CMD_CHILDREN(HalWriteItems), "run device write cmds" },
        { "read", DALHAL_CMD_CHILDREN(HalReadItems), "run device read cmds" },
        { "config", DALHAL_CMD_CHILDREN(HalConfigItems), "hal config cmds" },
        { "scripts", DALHAL_CMD_CHILDREN(HalScriptItems), "script specific commands" },
        { "getAvailableGPIOs", Exec_Hal_GetAvailableGPIOs, "get a list of available GPIO on this target and their functions" },
        { "printDevices", Exec_Hal_PrintDevices, "print all current loaded devices" },
        { "printLog", Exec_Hal_PrintLog, "print log" },
        { "printRegistry", Exec_Hal_PrintRegistry, "print device type registry" },
        { "printJsonSchemas", Exec_Hal_PrintJsonSchemas, "print config json schema" }
    };
#if defined(ESP8266) || defined(ESP32)
    static constexpr CommandNode WiFiItems[] = {
        { "scan", Exec_WiFi_Scan, "Scan WiFi for AP:s" },
        { "set_b64", Exec_WiFi_Set_b64, "Set current WiFi settings from the given b64 encoded string" },
        { "set_json", Exec_WiFi_Set_Json, "Set current WiFi settings from the given json encoded string" },
    };
#endif
    static constexpr CommandNode SystemItems[] = {
        
        { "info", Exec_System_Info, "System info" },
#if defined(ESP8266) || defined(ESP32)
        { "heap", Exec_System_Heap, "Print heap info" },
        { "reset", Exec_System_Reset_Restart, "Reset the system" },
        { "restart", Exec_System_Reset_Restart, "Restart the system (same as reset)" },
#endif
        { "HeartbeatLed", Exec_System_HeartbeatLed, "HeartBeatLed set timings" },
        { "ver", Exec_System_Build_Ver_Print, "Print current build version" },
    };

    static constexpr CommandNode RootItems[] = {
        { "hal", DALHAL_CMD_CHILDREN(HalItems), "HAL subsystem" },
        { "system", DALHAL_CMD_CHILDREN(SystemItems), "System commands" },
#if defined(ESP8266) || defined(ESP32)
        { "wifi", DALHAL_CMD_CHILDREN(WiFiItems), "WiFi management" },
#endif
        { "schedule", Exec_Scheduler_Cmd, "Schedule commands" }, // to be removed in the future as it would be implemented as a device
        { "help", Exec_Help_Cmd, "Show help" }
    };

    static constexpr CommandNode RootItem = {
        "", DALHAL_CMD_CHILDREN(RootItems), "root item"
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
        sbs.write_jsonString(F("name"), node.name);

        sbs.write_json_value_separator();

        // help
        sbs.write_jsonString(F("help"), node.help ? node.help : "");

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

            if (next.EqualsIC(child.name)) {

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
        } else {
            res = outDevice->exec(); // try fallback
        }

        if (res != HALOperationResult::Success) {
            GlobalLogger.Error(F("HALOperationResult: "), String(HALOperationResultToString(res)).c_str());
            GlobalLogger.setLastEntrySource(outDevice->Type);
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
            // first try fallback
            opres = device->read(val);
            if (opres == HALOperationResult::Success) {
                sbs.write_jsonNumber(F("value"), val.toUInt());
            } else {
                sbs.write_jsonMemberStart(F("error"));
                sbs.write_jsonQuoted(HALOperationResultToString(opres));
            }
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
            // first try fallback
            opres = device->read(val);
            if (opres == HALOperationResult::Success) {
                sbs.write_jsonNumber(F("value"), val.toInt());
            } else {
                sbs.write_jsonMemberStart(F("error"));
                sbs.write_jsonQuoted(HALOperationResultToString(opres));
            }
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
            // first try fallback
            opres = device->read(val);
            if (opres == HALOperationResult::Success) {
                sbs.write_jsonNumber(F("value"), val.toBool());
            } else {
                sbs.write_jsonMemberStart(F("error"));
                sbs.write_jsonQuoted(HALOperationResultToString(opres));
            }
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
            // first try fallback
            opres = device->read(val);
            if (opres == HALOperationResult::Success) {
                sbs.write_jsonNumber(F("value"), val.toFloat());
            } else {
                sbs.write_jsonMemberStart(F("error"));
                sbs.write_jsonQuoted(HALOperationResultToString(opres));
            }
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

            if (params.zcCmd.IsEmpty()) {
                opres = device->write(halValue);
            } else {
                auto fnRes = GetDeviceFunction<FunctionTypes::WriteHALValue>(device, params.zcCmd);
                if (fnRes.result == HALOperationResult::Success) {
                    opres = fnRes.fn(device, halValue);
                } else {
                    opres = fnRes.result;
                }
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

            if (params.zcCmd.IsEmpty()) {
                opres = device->write(halValue);
            } else {
                auto fnRes = GetDeviceFunction<FunctionTypes::WriteHALValue>(device, params.zcCmd);
                if (fnRes.result == HALOperationResult::Success) {
                    opres = fnRes.fn(device, halValue);
                } else {
                    opres = fnRes.result;
                }
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

            if (params.zcCmd.IsEmpty()) {
                opres = device->write(halValue);
            } else {
                auto fnRes = GetDeviceFunction<FunctionTypes::WriteHALValue>(device, params.zcCmd);
                if (fnRes.result == HALOperationResult::Success) {
                    opres = fnRes.fn(device, halValue);
                } else {
                    opres = fnRes.result;
                }
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

            if (params.zcCmd.IsEmpty()) {
                opres = device->write(halValue);
            } else {
                auto fnRes = GetDeviceFunction<FunctionTypes::WriteHALValue>(device, params.zcCmd);
                if (fnRes.result == HALOperationResult::Success) {
                    opres = fnRes.fn(device, halValue);
                } else {
                    opres = fnRes.result;
                }
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
        if (ScriptEngine::ValidateAndLoadAllActiveScripts()) {
            return HALOperationResult::Success;
        }
        return HALOperationResult::ExecutionFailed;
    }
    HALOperationResult Exec_Hal_Scripts_Stop(ZeroCopyString& zcStr, CommandCallback cb) {
        ScriptEngine::ScriptsBlock::running = false;
        return HALOperationResult::Success;
    }
    HALOperationResult Exec_Hal_Scripts_Start(ZeroCopyString& zcStr, CommandCallback cb) {
        ScriptEngine::ScriptsBlock::running = true;
        return HALOperationResult::Success;
    }

    HALOperationResult Exec_Hal_GetAvailableGPIOs(ZeroCopyString& zcStr, CommandCallback cb) {
        BlockStreamer bs(cb, "getAvailableGPIOs", BlockStreamer::DataType::Json);
        StringBuilderStreamer& sbs = bs.writer();
        GPIO_manager::GetList(zcStr, sbs);
        return HALOperationResult::Success;
    }
    HALOperationResult Exec_Hal_PrintDevices(ZeroCopyString& zcStr, CommandCallback cb) {
        BlockStreamer bs(cb, "printDevices", BlockStreamer::DataType::Json);
        StringBuilderStreamer& sbs = bs.writer();

        sbs.write_json_object_begin();
        DeviceManager::PrintTo(sbs);
        sbs.write_json_object_end();
        return HALOperationResult::Success;
    }
    HALOperationResult Exec_Hal_PrintLog(ZeroCopyString& zcStr, CommandCallback cb) {
        BlockStreamer bs(cb, "logs", BlockStreamer::DataType::PlainText);
        StringBuilderStreamer& sbs = bs.writer();
        GlobalLogger.printAllLogs(sbs);
        return HALOperationResult::Success;
    }
    HALOperationResult Exec_Hal_PrintRegistry(ZeroCopyString& zcStr, CommandCallback cb) {
        Registry::PrintTo(RootDevicesRegistry, cb);
        return HALOperationResult::Success;
    }
    HALOperationResult Exec_Hal_PrintJsonSchemas(ZeroCopyString& zcStr, CommandCallback cb) {
        JsonSchema::ToJsonString::buildCompleteJsonSchemasStartingFrom(RootDevicesRegistry, cb);
        return HALOperationResult::Success;
    }
}