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

#include <DALHAL/Support/DALHAL_Logger.h>

#include <BUILD_INFO.h>
#include <DALHAL/Support/base64.h>

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>

#include <DALHAL/Core/Manager/DALHAL_GPIO_Manager.h>
#include <DALHAL/Core/Manager/DALHAL_DeviceManager.h>
#include <DALHAL/Core/Types/DALHAL_Registry.h>
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

    CommandExecutor::ReadWriteCmdParameters::ReadWriteCmdParameters(ZeroCopyString& zcStr) {
        zcType = zcStr.SplitOffHead('/');
        zcUid = zcStr.SplitOffHead('/');
        zcValue = zcStr; // the value is the rest
    }
    
    bool CommandExecutor::execute(ZeroCopyString& zcStr, CommandCallback cb) {

        ZeroCopyString zcCommand = zcStr.SplitOffHead('/');

        bool anyErrors = false;
        if (zcCommand.EqualsIC(F("hal"))) {
            zcCommand = zcStr.SplitOffHead('/');
            if (zcCommand.EqualsIC(F(DALHAL_CMD_EXEC_WRITE_CMD))) {
                anyErrors = writeCmd(zcStr, cb) == false;
            }
            else if (zcCommand.EqualsIC(F(DALHAL_CMD_EXEC_READ_CMD))) {
                anyErrors = readCmd(zcStr, cb) == false;
            }
            else if (zcCommand.EqualsIC(F(DALHAL_CMD_EXEC_CMD))) {
                anyErrors = execCmd(zcStr, cb) == false;
            }
            else if (zcCommand.EqualsIC(F(DALHAL_CMD_EXEC_RELOAD_CFG_JSON))) {
                //long startMillis = millis();
                anyErrors = reloadJSON(zcStr, cb) == false;
                
                //printf("\n reloadJSON time:%ld ms\n", millis() - startMillis);
                //startMillis = millis();
                if (anyErrors == false) {
                    
                    anyErrors = ScriptEngine::ValidateAndLoadAllActiveScripts() == false;
                }
                //printf("\n ValidateAndLoadAllActiveScripts time:%ld ms\n", millis() - startMillis);
            }
            else if (zcCommand.EqualsIC(F("scripts"))) {
                ZeroCopyString zcSubCmd = zcStr.SplitOffHead('/');
                if (zcSubCmd.EqualsIC(F("reload"))) {
                    //long startMillis = millis();
                    anyErrors = ScriptEngine::ValidateAndLoadAllActiveScripts() == false;
                    //printf("\nValidateAndLoadAllActiveScripts time:%ld ms\n", millis() - startMillis);
                } else if (zcSubCmd.EqualsIC(F("stop"))) {
                    ScriptEngine::ScriptsBlock::running = false;
                } else if (zcSubCmd.EqualsIC(F("start"))) {
                    ScriptEngine::ScriptsBlock::running = true;
                } else {
                    anyErrors = true;
                    GlobalLogger.Error(F("Unknown scripts subcommand: "), zcSubCmd);
                    //message += "\"error\":\"Unknown scripts subcommand.\"";
                    //message += ",\"command\":\""+zcSubCmd.ToString()+"\"";
                }
                if (anyErrors == false) {
                    BlockStreamer bs(cb, "scripts exec", BlockStreamer::DataType::Json);
                    StringBuilderStreamer& sbs = bs.writer();
                    sbs.write(F("\"info\":\"OK\""));
                }
            }
            else if (zcCommand.EqualsIC(F("getAvailableGPIOs"))) {
                BlockStreamer bs(cb, "getAvailableGPIOs", BlockStreamer::DataType::Json);
                StringBuilderStreamer& sbs = bs.writer();
                GPIO_manager::GetList(zcStr, sbs);
            }
            else if (zcCommand.EqualsIC(F("printDevices"))) {
                BlockStreamer bs(cb, "scripts exec", BlockStreamer::DataType::Json);
                StringBuilderStreamer& sbs = bs.writer();
                // TODO remove use of std::string here
                std::string tmp = DeviceManager::ToString();
                sbs.write(tmp.c_str(), tmp.length());
            }
            else if (zcCommand.EqualsIC(F("printLog"))) {

                if (cb != nullptr) {
                    // TODO remove use of std::string here
                    GlobalLogger.printAllLogs([cb](const std::string& msg){
                        const ZeroCopyString zcMsg = msg.c_str();
                        cb(zcMsg, CmdCbType::Control);
                    });
                }

            }
            else if (zcCommand.EqualsIC(F("printRegistry"))) {
                if (cb != nullptr) {
                    Registry::PrintTo(RootDevicesRegistry, cb);
                }
            }
            else if (zcCommand.EqualsIC(F("printJsonSchemas"))) {
                if (cb != nullptr) {
                    JsonSchema::ToJsonString::buildCompleteJsonSchemasStartingFrom(RootDevicesRegistry, cb);
                }
            }
            else
            {
                anyErrors = true;
                GlobalLogger.Error(F("Unknown HAL command: "), zcCommand);
            }
        } 
#if defined(ESP8266) || defined(ESP32)
        else if (zcCommand.EqualsIC(F("wifi"))) {
            zcCommand = zcStr.SplitOffHead('/');

            if (zcCommand.EqualsIC(F("scan"))) {
                
                BlockStreamer bs(cb, "wifi/found", BlockStreamer::DataType::Json);
                StringBuilderStreamer& sbs = bs.writer();
                sbs.write('[');
                int n = WiFi.scanNetworks(false, true);
                for (int i = 0; i < n; i++) {
                    if (i > 0) {
                        sbs.write(',');
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
                    sbs.write('{');
                    sbs.write_jsonString(F("ssid"), ssidB64.c_str());
                    sbs.write(',');
                    sbs.write_jsonNumber(F("ch"), WiFi.channel(i));
                    sbs.write(',');
                    sbs.write_jsonNumber(F("freq"), WiFi.channel(i) <= 14 ? 2400 : 5000); // crude 2.4/5 GHz
                    sbs.write(',');
                    sbs.write_jsonNumber(F("rssi"), WiFi.RSSI(i));
                    sbs.write(',');
                    sbs.write_jsonString(F("encryption"), enc);
                    sbs.write('}');
                }
                sbs.write(']');
                
            } else if (zcCommand.EqualsIC(F("set_b64"))) {
                if (zcStr.CountChar(':') >= 1) {
                    DALHAL::ZeroCopyString zcSSID = zcStr.SplitOffHead(':');
                    char ssid[33] = {0};
                    char pass[65] = {0};

                    int ssidLen = b64urlDecode((uint8_t*)ssid, sizeof(ssid) - 1, zcSSID);
                    int passLen = b64urlDecode((uint8_t*)pass, sizeof(pass) - 1, zcStr);
                    if (ssidLen <= 0 || passLen < 0) {
                        if (cb != nullptr) {

                            if (ssidLen == 0) {
                                cb(String(F("wifi/set_b64/error/ssid/empty\r\n")).c_str(), CmdCbType::Data);
                            } else if (ssidLen == -1) {
                                cb(String(F("wifi/set_b64/error/ssid/invalidchar\r\n")).c_str(), CmdCbType::Data);
                            } else if (ssidLen == -2) {
                                cb(String(F("wifi/set_b64/error/ssid/long\r\n")).c_str(), CmdCbType::Data);
                            }

                            if (passLen == -1) {
                                cb(String(F("wifi/set_b64/error/pass/invalidchar\r\n")).c_str(), CmdCbType::Data);
                            } else if (passLen == -2) {
                                cb(String(F("wifi/set_b64/error/pass/long\r\n")).c_str(), CmdCbType::Data);
                            }
                        }
                        return false;
                    }

                    if (cb != nullptr) {
                        cb(String(F("wifi/set_b64/OK\r\n")).c_str(), CmdCbType::Data);
                        delay(50);
                    }
                    
                    SetWifiCredentialsAndRestart(ssid, pass);

                } else {
                    if (cb != nullptr) {
                        cb(String(F("wifi/set_b64/error/missingparams\r\n")).c_str(), CmdCbType::Data);
                    }
                }
            } else if (zcCommand.EqualsIC(F("set_json"))) {
                
                DynamicJsonDocument doc(256);
                DeserializationError err = deserializeJson(doc, zcStr.start); // safe to use start here as that string is null terminated
                if(err) {
                    if(cb) cb(String(F("wifi/set_json/error/invalidjson\r\n")).c_str(), CmdCbType::Data);
                    return false;
                }

                const char* ssid = doc["ssid"];
                const char* pass = doc["pass"];

                if(ssid == nullptr || ssid[0] == '\0') {
                    if(cb) { cb(String(F("wifi/set_json/error/ssid/empty\r\n")).c_str(), CmdCbType::Data); }
                    return false;
                }

                if(pass == nullptr) pass = ""; // allow empty password if desired

                if(cb) { cb(String(F("wifi/set_json/OK\r\n")).c_str(), CmdCbType::Data); }

                SetWifiCredentialsAndRestart(ssid, pass);
            }
        } 
#endif
        else if (zcCommand.EqualsIC(F("system"))) {
            zcCommand = zcStr.SplitOffHead('/');
            if (zcCommand.EqualsIC(F("info"))) {
                if (cb != nullptr) {
#if defined(ESP8266) || defined(ESP32)
                    std::string infoMsg = Info::getESP_info();
#else
                    std::string infoMsg = "{\"mcu\":\"windows\"}";
#endif
                    cb(infoMsg.c_str(), CmdCbType::Data);
                }
            } 
#if defined(ESP8266) || defined(ESP32)
            else if (zcCommand.EqualsIC(F("heap"))) {
                if (cb != nullptr) {
                    BlockStreamer bs(cb, "heap", BlockStreamer::DataType::Json);
                    StringBuilderStreamer& sbs = bs.writer();
                    sbs.write('{');
                    sbs.write_jsonNumber(F("Heap"), ESP.getFreeHeap());
#if defined(ESP8266)
                    sbs.write(','); sbs.write_jsonNumber(F("Max block"), ESP.getMaxFreeBlockSize());
#elif defined(ESP32)
                    sbs.write(','); sbs.write_jsonNumber(F("Max block"), ESP.getMaxAllocHeap());
#endif
                    sbs.write('}');
                }
            }
            else if (zcCommand.EqualsIC(F("reset")) || zcCommand.EqualsIC(F("restart"))) {
                if (cb != nullptr) {
                    BlockStreamer bs(cb, "reset", BlockStreamer::DataType::PlainText);
                    StringBuilderStreamer& sbs = bs.writer();
                    sbs.write(F("the system will now restart"));
                    sbs.flush();
                }
                ESP.restart();
            }
#endif

            else if (zcCommand.EqualsIC(F("HeartbeatLed"))) {
                BlockStreamer bs(cb, "HeartbeatLed", BlockStreamer::DataType::PlainText);
                StringBuilderStreamer& sbs = bs.writer();
                HeartbeatLed::parseCmd(zcStr, sbs);
                
            }
            else {
                if (cb != nullptr) {
                    BlockStreamer bs(cb, "system", BlockStreamer::DataType::PlainText);
                    StringBuilderStreamer& sbs = bs.writer();
                    sbs.write(F("error system sub command not found"));
                }
            }
        } else if (zcCommand.EqualsIC(F("schedule"))) {

            std::string resStr;
            Scheduler::parseCmd(zcStr, resStr);
            if (cb != nullptr) {
                const ZeroCopyString zcMsg = resStr.c_str();
                cb(zcMsg, CmdCbType::Data);
            }
        } else if (zcCommand.EqualsIC(F("ver"))) {
            BlockStreamer bs(cb, "ver", BlockStreamer::DataType::Json);
            StringBuilderStreamer& sbs = bs.writer();
            sbs.write(F("{\"build_version\":\"" BUILD_VER_STR "\"}"));
        } else if (zcCommand.EqualsIC(F("help"))) {
            BlockStreamer bs(cb, "help", BlockStreamer::DataType::Json);
            StringBuilderStreamer& sbs = bs.writer();
            sbs.write('{');
            if (zcStr.IsEmpty()) {
                sbs.write_jsonKey(F("available_root_cmds"));
                sbs.write('[');
                sbs.write(F("\"help\",\"ver\",\"hal\",\"wifi\""));
                sbs.write(']');
            } else {
                zcCommand = zcStr.SplitOffHead('/');
                sbs.write_jsonKey(F("info"));
                if (zcCommand.EqualsIC(F("ver"))) {
                    sbs.write_jsonQuoted(F("shows build version, can be used to determine what specific ver a device runs"));
                } else if (zcCommand.EqualsIC(F("hal"))) {
                    sbs.write_jsonQuoted(F("write read exec reloadcfg printdevices scripts printlog getavailablegpios"));
                } else if (zcCommand.EqualsIC(F("wifi"))) {
                    sbs.write_jsonQuoted(F("scan set"));
                } else {
                    sbs.write_jsonQuoted(F("!!!! ERROR NO HELP !!!!"));
                }

            }
            sbs.write('}');
        }
        else
        {
            anyErrors = true;
            GlobalLogger.Error(F("Unknown root command: "), zcCommand);
        }
        if (anyErrors) {

            const LogEntry& lastEntry = GlobalLogger.getLastEntry();
            if (lastEntry.level == Loglevel::Error) {
                BlockStreamer bs(cb, "help", BlockStreamer::DataType::Json);
                StringBuilderStreamer& sbs = bs.writer();
                sbs.write('{');
                sbs.write_jsonKey(F("error"));
                lastEntry.MessageWriteTo(sbs);
                sbs.write('}');

            }
        }
        /*if (message.length() != 0) { // this lets the command exec print it's own format
            message = '{' + message;
            message += '}';
            if (cb != nullptr) {

                const ZeroCopyString zcMsg(message.c_str(), message.length());
                cb(zcMsg, CmdCbType::Data);
            }
        }*/
        
        return (anyErrors == false);
    }
    bool CommandExecutor::reloadJSON(ZeroCopyString& zcStr, CommandCallback cb) {
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
    //  ██     ██ ██████  ██ ████████ ███████ 
    //  ██     ██ ██   ██ ██    ██    ██      
    //  ██  █  ██ ██████  ██    ██    █████   
    //  ██ ███ ██ ██   ██ ██    ██    ██      
    //   ███ ███  ██   ██ ██    ██    ███████ 
    // 

    bool CommandExecutor::writeCmd(ZeroCopyString& zcStr, CommandCallback cb) {
        ReadWriteCmdParameters params(zcStr);

        // check if have uid given
        if (params.zcUid.IsEmpty()) {
            GlobalLogger.Error(F("uid path is empty"));

            return false;
        }
        // check if device exists
        UIDPath uidPath(params.zcUid);
        Device* outDevice = nullptr;
        DeviceFindResult devFindRes = DeviceManager::findDevice(uidPath, outDevice);
        if (devFindRes != DeviceFindResult::Success) {
            GlobalLogger.Error(F("device not found: "), params.zcUid);
            GlobalLogger.setLastEntrySource(DeviceFindResultToString(devFindRes));

            return false;
        }
        // write need a value
        if (params.zcValue.Length() == 0) {
            GlobalLogger.Error(F("No value provided for writing."));

            return false;
        }

        HALOperationResult writeResult = HALOperationResult::UnsupportedOperation;

        BlockStreamer bs(cb, "hal/write", BlockStreamer::DataType::Json);
        StringBuilderStreamer& sbs = bs.writer();
        sbs.write('{');
        sbs.write_jsonKey(F("info"));
        bool anyError = false;

        if (params.zcType == DALHAL_CMD_EXEC_BOOL_TYPE) {
            uint32_t uintValue = 0;

            if (params.zcValue.EqualsIC(F("true")) || params.zcValue.Equals('1')) {
                uintValue = 1;
            } else if (params.zcValue.EqualsIC(F("false")) || params.zcValue.Equals('0')) {
                uintValue = 0;
            } else {

                GlobalLogger.Error(F("Invalid boolean value."));
                sbs.write(F("null"));
                sbs.write('}');
                return false;
            }

            HALValue halValue = uintValue;

            writeResult = outDevice->write(halValue);
            if (writeResult == HALOperationResult::Success) {
                sbs.write('{');
                sbs.write_jsonNumber(F("Value written"), uintValue);
                sbs.write('}');
                
            }
        }
        else if (params.zcType.EqualsIC(F(DALHAL_CMD_EXEC_UINT32_TYPE))) {
            // Convert value to integer
            uint32_t uintValue = 0;
            if (params.zcValue.ConvertTo_uint32(uintValue) == false) {
                GlobalLogger.Error(F("Invalid uint32 value."));

            } else {

                HALValue halValue = uintValue;

                writeResult = outDevice->write(halValue);
                if (writeResult == HALOperationResult::Success) {
                    sbs.write('{');
                    sbs.write_jsonNumber(F("Value written"), uintValue);
                    sbs.write('}');
                }
            }

        } else if (params.zcType.EqualsIC(F(DALHAL_CMD_EXEC_FLOAT_TYPE))) {
            // Convert value to integer
            float floatValue = 0.0f;
            if (params.zcValue.ConvertTo_float(floatValue) == false) {
                GlobalLogger.Error(F("Invalid float value."));
            } else {

                HALValue halValue = floatValue;

                writeResult = outDevice->write(halValue);
                if (writeResult == HALOperationResult::Success) {
                    
                    sbs.write('{');
                    sbs.write_jsonNumber(F("Value written"), floatValue);
                    sbs.write('}');
                }
            }

        } else if (params.zcType.EqualsIC(F(DALHAL_CMD_EXEC_STRING_TYPE))) {
            HALWriteStringRequestValue strHalValue(params.zcValue, sbs);

            writeResult = outDevice->write(strHalValue);
            if (writeResult == HALOperationResult::Success) {
                sbs.write('{');
                sbs.write_jsonString(F("String written"), params.zcValue);
                sbs.write('}');
            }

        } else if (params.zcType.EqualsIC(F(DALHAL_CMD_EXEC_JSON_STR_TYPE))) {
            
            HALWriteStringRequestValue strHalValue(params.zcValue, sbs);

            writeResult = outDevice->write(strHalValue);
            if (writeResult == HALOperationResult::Success) {
                sbs.write('{');
                sbs.write_jsonString(F("Json string written"), params.zcValue);
                sbs.write('}');
            }
        }
        else {
            GlobalLogger.Error(F("Unknown type for writing."));
            anyError = true;
        }
        if (anyError == false && writeResult != HALOperationResult::Success) {
            GlobalLogger.Error(F("HALOperationResult: "), String(HALOperationResultToString(writeResult)).c_str());
            GlobalLogger.setLastEntrySource(outDevice->Type);
            anyError = true;
        }
        sbs.write('}');
        return anyError == false;
    }
    //  ██████  ███████  █████  ██████  
    //  ██   ██ ██      ██   ██ ██   ██ 
    //  ██████  █████   ███████ ██   ██ 
    //  ██   ██ ██      ██   ██ ██   ██ 
    //  ██   ██ ███████ ██   ██ ██████  

    bool CommandExecutor::readCmd(ZeroCopyString& zcStr, CommandCallback cb) {
        ReadWriteCmdParameters params(zcStr);
                
        // check if have uid given
        if (params.zcUid.IsEmpty()) {
            GlobalLogger.Error(F("uid path is empty"));
            return false;
        }
        // check if device exists
        UIDPath uidPath(params.zcUid);
        Device* outDevice = nullptr;
        DeviceFindResult devFindRes = DeviceManager::findDevice(uidPath, outDevice);
        if (devFindRes != DeviceFindResult::Success) {
            GlobalLogger.Error(F("device not found: "), params.zcUid);
            GlobalLogger.setLastEntrySource(DeviceFindResultToString(devFindRes));
            return false;
        }
        HALOperationResult readResult = HALOperationResult::UnsupportedOperation;

        BlockStreamer bs(cb, "hal/read", BlockStreamer::DataType::Json);
        StringBuilderStreamer& sbs = bs.writer();
        sbs.write('{');
        sbs.write_jsonKey(F("value"));

        bool anyError = false;

        if (params.zcType.EqualsIC(F(DALHAL_CMD_EXEC_BOOL_TYPE))) {
            HALValue halValue;

            readResult = outDevice->read(halValue);

            if (readResult == HALOperationResult::Success) {
                sbs.write(halValue.toUInt());
            }
        } else if (params.zcType.EqualsIC(F(DALHAL_CMD_EXEC_UINT32_TYPE))) {
            HALValue halValue;

            readResult = outDevice->read(halValue);

            if (readResult == HALOperationResult::Success) {
                sbs.write(halValue.toUInt());
            }
        } else if (params.zcType.EqualsIC(F(DALHAL_CMD_EXEC_FLOAT_TYPE))) {

            if (params.zcValue.Length() == 0) {
                HALValue halValue;

                readResult = outDevice->read(halValue);

                if (readResult == HALOperationResult::Success) {
                    sbs.write(halValue.toFloat());
                }
            }
            else {
                HALValue halValue;
                HALReadValueByCmd valByCmd(halValue, params.zcValue);

                readResult = outDevice->read(valByCmd);

                if (readResult == HALOperationResult::Success) {
                    sbs.write(halValue.toFloat());
                }
            }
        } else if (params.zcType.EqualsIC(F(DALHAL_CMD_EXEC_STRING_TYPE))) {
            
            HALReadStringRequestValue strHalValue(params.zcValue, sbs);

            readResult = outDevice->read(strHalValue); // the result if any is directly written to sbs

        } else if (params.zcType.EqualsIC(F(DALHAL_CMD_EXEC_JSON_STR_TYPE))) {

            HALReadStringRequestValue strHalValue(params.zcValue, sbs);
  
            readResult = outDevice->read(strHalValue); // the result if any is directly written to sbs

        } else {
            GlobalLogger.Error(F("Unknown type for reading."));
            sbs.write(F("null"));
            anyError = true;
        }
        if (anyError == false && readResult != HALOperationResult::Success) {
            GlobalLogger.Error(F("HALOperationResult: "), String(HALOperationResultToString(readResult)).c_str());
            GlobalLogger.setLastEntrySource(outDevice->Type);
            sbs.write(F("null"));
            anyError = true;
        }

        sbs.write('}');
        return anyError == false;
    }

    bool CommandExecutor::execCmd(ZeroCopyString& zcStr, CommandCallback cb) {
        ZeroCopyString zcPath = zcStr.SplitOffHead('/');
        // check if have uid given
        if (zcPath.IsEmpty()) {
            GlobalLogger.Error(F("uid path empty"));
            return false;
        }
        // first check if device exists
        UIDPath uidPath(zcPath);
        Device* outDevice = nullptr;
        DeviceFindResult devFindRes = DeviceManager::findDevice(uidPath, outDevice);
        if (devFindRes != DeviceFindResult::Success) {
            GlobalLogger.Error(F("device not found: "), zcPath);
            GlobalLogger.setLastEntrySource(DeviceFindResultToString(devFindRes));
            return false;
        }

        HALOperationResult res = HALOperationResult::NotSet;
        if (zcStr.NotEmpty()) {
            res = outDevice->exec(zcStr);
        } else {
            res = outDevice->exec();
        }
        if (res != HALOperationResult::Success) {
            GlobalLogger.Error(F("HALOperationResult: "), String(HALOperationResultToString(res)).c_str());
            GlobalLogger.setLastEntrySource(outDevice->Type);
            return false;
        }
        return true;
    }
#if defined(ESP8266) || defined(ESP32)
    void CommandExecutor::SetWifiCredentialsAndRestart(const char* ssid, const char* pass) {
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
}