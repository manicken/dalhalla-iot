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

#include "HAL_JSON_CommandExecutor.h"

#include "../Support/Logger.h"
#include "../Support/ConvertHelper.h"

#include "HAL_JSON_Device_GlobalDefines.h"

#include "HAL_JSON_GPIO_Manager.h"
#include "HAL_JSON_Manager.h"
#include "ScriptEngine/HAL_JSON_SCRIPT_ENGINE.h"

#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
#include <iostream>
#endif

namespace HAL_JSON {

#if defined(ESP32) || defined(ESP8266)
    portMUX_TYPE CommandExecutor::g_pendingMux = portMUX_INITIALIZER_UNLOCKED;
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
#ifdef HAL_JSON_CommandExecutor_DEBUG_CMD
    std::string CommandExecutor::ReadWriteCmdParameters::ToString() {
        std::string message;
        message += "\"Type\":\"" + zcType.ToString() + "\",";
        message += "\"UID\":\"" + zcUid.ToString() + "\",";
        message += "\"Value\":\"" + zcValue.ToString() + "\"";
        return message;
    }
#endif
    
    // TODO: refactor this function to send errors to GlobalLogger
    bool CommandExecutor::execute(ZeroCopyString& zcStr, std::string& message) {
        // Example URL: /write/uint32/tempSensor1/255 where tempSensor1 is a uid defined by cfg json
        //std::cout << "zcStr:" << zcStr.ToString() << "\n";
        ZeroCopyString zcCommand = zcStr.SplitOffHead('/');

        //std::cout << "zcCommand:" << zcCommand.ToString() << "\n";

#ifdef HAL_JSON_CommandExecutor_DEBUG_CMD
        message += "\"debug\":{";
        message += "\"Command\":\"" + zcCommand.ToString() + "\",";
#endif
        //bool addLastLogEntryToMessage = false;
        bool anyErrors = false;
        if (zcCommand == HAL_JSON_CMD_EXEC_WRITE_CMD) {
            anyErrors = writeCmd(zcStr, message) == false;
        }
        else if (zcCommand == HAL_JSON_CMD_EXEC_READ_CMD) {
            anyErrors = readCmd(zcStr, message) == false;
        }
        else if (zcCommand == HAL_JSON_CMD_EXEC_CMD) {
            anyErrors = execCmd(zcStr, message) == false;
        }
        else if (zcCommand == HAL_JSON_CMD_EXEC_RELOAD_CFG_JSON) {
            long startMillis = millis();
            anyErrors = reloadJSON(zcStr, message) == false;
            printf("\n reloadJSON time:%ld ms\n", millis() - startMillis);
            startMillis = millis();
            anyErrors = ScriptEngine::ValidateAndLoadAllActiveScripts() == false;
            printf("\n ValidateAndLoadAllActiveScripts time:%ld ms\n", millis() - startMillis);
        }
        else if (zcCommand == "scripts") {
            ZeroCopyString zcSubCmd = zcStr.SplitOffHead('/');
            if (zcSubCmd == "reload") {
                long startMillis = millis();
                anyErrors = ScriptEngine::ValidateAndLoadAllActiveScripts() == false;
                printf("\nValidateAndLoadAllActiveScripts time:%ld ms\n", millis() - startMillis);
            } else if (zcSubCmd == "stop") {
                ScriptEngine::ScriptsBlock::running = false;
            } else if (zcSubCmd == "start") {
                ScriptEngine::ScriptsBlock::running = true;
            } else {
                anyErrors = true;
                GlobalLogger.Error(F("Unknown scripts subcommand: "), zcSubCmd);
                //message += "\"error\":\"Unknown scripts subcommand.\"";
                //message += ",\"command\":\""+zcSubCmd.ToString()+"\"";
            }
            if (anyErrors == false)
                message += "\"info\":\"OK\"";
        }
        else if (zcCommand == HAL_JSON_CMD_EXEC_GET_AVAILABLE_GPIO_LIST) {
            message += GPIO_manager::GetList(zcStr);
        }
        else if (zcCommand == HAL_JSON_CMD_EXEC_PRINT_DEVICES) {
            message += Manager::ToString();
        }
        else if (zcCommand == HAL_JSON_CMD_EXEC_PRINT_LOG_CONTENTS) {
            GlobalLogger.printAllLogs(Serial);
        }
        else
        {
            anyErrors = true;
            GlobalLogger.Error(F("Unknown command: "), zcCommand);
            //message += "\"error\":\"Unknown command.\"";
            //message += ",\"command\":\""+zcCommand.ToString()+"\"";
        }
        if (anyErrors) {

            const LogEntry& lastEntry = GlobalLogger.getLastEntry();
            if (lastEntry.level == Loglevel::Error) {
                String lastEntryStr = lastEntry.MessageToString();
                message += "\"error\":\"";
                message += lastEntryStr.c_str();
                message += "\"";
            }
        }
        message = "{" + message;
        message += "}";
        
        return (anyErrors == false);
    }
    bool CommandExecutor::reloadJSON(ZeroCopyString& zcStr, std::string& message) {
        ZeroCopyString zcOptionalFileName = zcStr.SplitOffHead('/');
#ifdef HAL_JSON_CommandExecutor_DEBUG_CMD
        message += "\"filename\":\"" + (zcOptionalFileName.NotEmpty()?zcOptionalFileName.ToString():"default") + "\"}";
#endif
        std::string filePath;
#if defined(ESP32) || defined(ESP8266)
        filePath = "/";
#endif
        if (zcOptionalFileName.Length() == 0) {
            filePath += "hal/cfg.json";
        } else {
            filePath += "hal/" + zcOptionalFileName.ToString();
        }

#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
        std::cout << "Reload cfg json: " << filePath << std::endl;  
#endif
    
        if (Manager::ReadJSON(filePath.c_str())) {
            message += "\"info\":\"OK\"";
            return true;
        } else {
            message += "\"info\":\"FAIL\",";
            return false;
        }
    }
    //  ██     ██ ██████  ██ ████████ ███████ 
    //  ██     ██ ██   ██ ██    ██    ██      
    //  ██  █  ██ ██████  ██    ██    █████   
    //  ██ ███ ██ ██   ██ ██    ██    ██      
    //   ███ ███  ██   ██ ██    ██    ███████ 
    // 

    // TODO: refactor this function to send errors to GlobalLogger
    bool CommandExecutor::writeCmd(ZeroCopyString& zcStr, std::string& message) {
        ReadWriteCmdParameters params(zcStr);
#ifdef HAL_JSON_CommandExecutor_DEBUG_CMD
        message += params.ToString() + "},";
#endif
        // check if have uid given
        if (params.zcUid.IsEmpty()) {
            GlobalLogger.Error(F("uid path is empty"));
            //message += "\"error\":\"uid path is empty\"";
            return false;
        }
        // check if device exists
        UIDPath uidPath(params.zcUid);
        Device* outDevice = nullptr;
        DeviceFindResult devFindRes = Manager::findDevice(uidPath, outDevice);
        if (devFindRes != DeviceFindResult::Success) {
            GlobalLogger.Error(F("device not found: "), params.zcUid);
            GlobalLogger.setLastEntrySource(DeviceFindResultToString(devFindRes));
            //message += "\"error\":\"device not found\"";
            //message += ",\"uidpath\":\"" + params.zcUid.ToString() + "\"";
            return false;
        }
        // write need a value
        if (params.zcValue.Length() == 0) {
            GlobalLogger.Error(F("No value provided for writing."));
            //message += "\"error\":\"No value provided for writing.\"";
            return false;
        }

        HALOperationResult writeResult = HALOperationResult::UnsupportedOperation;

        if (params.zcType == HAL_JSON_CMD_EXEC_BOOL_TYPE) {
            uint32_t uintValue = 0;

            if ((params.zcValue == "true") || (params.zcValue == "1")) {
                uintValue = 1;
            } else if ((params.zcValue == "false") || (params.zcValue == "0")) {
                uintValue = 0;
            } else {

                //message += "{\"error\":\"Invalid boolean value.\"}";
                GlobalLogger.Error(F("Invalid boolean value."));
                return false;
            }

            //UIDPath uidPath(params.zcUid); // obsolete
            HALValue halValue = uintValue;
            //HALWriteRequest req(uidPath, halValue); // obsolete

            writeResult = outDevice->write(halValue);
            if (writeResult == HALOperationResult::Success) {
                message += "\"info\":{\"Value written\":\"";
                message += std::to_string(uintValue);
                message += "\"}";
            }
        }
        else if (params.zcType == HAL_JSON_CMD_EXEC_UINT32_TYPE) {
            // Convert value to integer
            uint32_t uintValue = 0;
            if (params.zcValue.ConvertTo_uint32(uintValue) == false) {
                GlobalLogger.Error(F("Invalid uint32 value."));
                //message += "{\"error\":\"Invalid uint32 value.\"}";
            } else {
                //UIDPath uidPath(params.zcUid); // obsolete
                HALValue halValue = uintValue;
                //HALWriteRequest req(uidPath, halValue); // obsolete

                writeResult = outDevice->write(halValue);
                if (writeResult == HALOperationResult::Success) {
                    message += "\"info\":{\"Value written\":\"";
                    message += std::to_string(uintValue);
                    message += "\"}";
                }
            }

        } else if (params.zcType == HAL_JSON_CMD_EXEC_FLOAT_TYPE) {
            // Convert value to integer
            float floatValue = 0.0f;
            if (params.zcValue.ConvertTo_float(floatValue) == false) {
                GlobalLogger.Error(F("Invalid float value."));
                //message += "{\"error\":\"Invalid float value.\"}";
            } else {
#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
                std::cout << "float value written: " << floatValue << "\n";
#endif
                //UIDPath uidPath(params.zcUid); // obsolete
                HALValue halValue = floatValue;
                //HALWriteRequest req(uidPath, halValue); // obsolete

                writeResult = outDevice->write(halValue);
                if (writeResult == HALOperationResult::Success) {
                    message += "\"info\":{\"Value written\":\"";
                    message += std::to_string(floatValue);
                    message += "\"}";
                }
            }

        } else if (params.zcType == HAL_JSON_CMD_EXEC_STRING_TYPE) {
            //UIDPath uidPath(params.zcUid);  // obsolete
            std::string result;
            HALWriteStringRequestValue strHalValue(params.zcValue, result);
            
            //HALWriteStringRequest req(uidPath, strHalValue);  // obsolete

            writeResult = outDevice->write(strHalValue);
            if (writeResult == HALOperationResult::Success) {
                message += "\"info\":{\"String written\":\"OK\"}";
                //message += stdString.c_str();
                //message += "\"}";
            }

        } else if (params.zcType == HAL_JSON_CMD_EXEC_JSON_STR_TYPE) {
            //UIDPath uidPath(params.zcUid);  // obsolete
            std::string result;
            HALWriteStringRequestValue strHalValue(params.zcValue, result);
            //HALWriteStringRequest req(uidPath, strHalValue);  // obsolete

            writeResult = outDevice->write(strHalValue);
            if (writeResult == HALOperationResult::Success) {
                message += "\"info\":{\"Json written\":\"OK\"}";
                //message += stdString.c_str();
                //message += "}";
            }
        }
        else {
            GlobalLogger.Error(F("Unknown type for writing."));
            //message += "\"error\":\"Unknown type for writing.\"";
            return false;
        }
        if (writeResult != HALOperationResult::Success) {
            GlobalLogger.Error(F("HALOperationResult: "), HALOperationResultToString(writeResult));
            GlobalLogger.setLastEntrySource(outDevice->GetType());
            /*message += "\"error\":\"";
            message += HALOperationResultToString(writeResult);
            message += '"';
            message += ",\"target_type\":\"";
            message += device->GetType();
            message += '"';*/
            return false;
        }
        return true;
    }
    //  ██████  ███████  █████  ██████  
    //  ██   ██ ██      ██   ██ ██   ██ 
    //  ██████  █████   ███████ ██   ██ 
    //  ██   ██ ██      ██   ██ ██   ██ 
    //  ██   ██ ███████ ██   ██ ██████  

    // TODO: refactor this function to send errors to GlobalLogger
    bool CommandExecutor::readCmd(ZeroCopyString& zcStr, std::string& message) {
        ReadWriteCmdParameters params(zcStr);
        std::string valueStr;
#ifdef HAL_JSON_CommandExecutor_DEBUG_CMD
        message += params.ToString() + "},";
#endif
        // check if have uid given
        if (params.zcUid.IsEmpty()) {
            GlobalLogger.Error(F("uid path is empty"));
            //message += "\"error\":\"uid path is empty\"";
            return false;
        }
        // check if device exists
        UIDPath uidPath(params.zcUid);
        Device* outDevice = nullptr;
        DeviceFindResult devFindRes = Manager::findDevice(uidPath, outDevice);
        if (devFindRes != DeviceFindResult::Success) {
            GlobalLogger.Error(F("device not found: "), params.zcUid);
            GlobalLogger.setLastEntrySource(DeviceFindResultToString(devFindRes));
            //message += "\"error\":\"device not found\"";
            //message += ",\"uidpath\":\"" + params.zcUid.ToString() + "\"";
            return false;
        }
        HALOperationResult readResult = HALOperationResult::UnsupportedOperation;

        if (params.zcType == HAL_JSON_CMD_EXEC_BOOL_TYPE) {
            //UIDPath uidPath(params.zcUid); // obsolete
            HALValue halValue;
            //HALReadRequest req(uidPath, halValue); // obsolete

            readResult = outDevice->read(halValue);
            if (readResult == HALOperationResult::Success) {
                valueStr = std::to_string(halValue.asUInt());
            }
        } else if (params.zcType == HAL_JSON_CMD_EXEC_UINT32_TYPE) {
            //UIDPath uidPath(params.zcUid); // obsolete
            HALValue halValue;
            //HALReadRequest req(uidPath, halValue); // obsolete

            readResult = outDevice->read(halValue);
            if (readResult == HALOperationResult::Success) {
                valueStr = std::to_string(halValue.asUInt());
            }
        } else if (params.zcType == HAL_JSON_CMD_EXEC_FLOAT_TYPE) {
            //UIDPath uidPath(params.zcUid); // obsolete
            if (params.zcValue.Length() == 0) {
                HALValue halValue;
                //HALReadRequest req(uidPath, halValue); // obsolete

                readResult = outDevice->read(halValue);
                if (readResult == HALOperationResult::Success) {
                    valueStr = std::to_string(halValue.asFloat());
                }
            }
            else {
                HALValue halValue;
                HALReadValueByCmd valByCmd(halValue, params.zcValue);
                //HALReadValueByCmdReq req(uidPath, valByCmd); // obsolete

                readResult = outDevice->read(valByCmd);
                if (readResult == HALOperationResult::Success) {
                    valueStr = std::to_string(halValue.asFloat());
                }
            }
        } else if (params.zcType == HAL_JSON_CMD_EXEC_STRING_TYPE) {
            //UIDPath uidPath(params.zcUid); // obsolete
            std::string result;
            HALReadStringRequestValue strHalValue(params.zcValue, result);
            //HALReadStringRequest req(uidPath, strHalValue); // obsolete

            readResult = outDevice->read(strHalValue);
            if (readResult == HALOperationResult::Success) {
                valueStr = "\"";
                valueStr += result;
                valueStr += "\"";
            }
        } else if (params.zcType == HAL_JSON_CMD_EXEC_JSON_STR_TYPE) {
            //UIDPath uidPath(params.zcUid); // obsolete
            std::string result;
            HALReadStringRequestValue strHalValue(params.zcValue, result);
            //HALReadStringRequest req(uidPath, strHalValue); // obsolete

            readResult = outDevice->read(strHalValue);
            if (readResult == HALOperationResult::Success) {
                valueStr += result;
            }
        } else {
            GlobalLogger.Error(F("Unknown type for reading."));
            //message += "\"error\":\"Unknown type for reading.\"";
            return false;
        }
        if (readResult != HALOperationResult::Success) {
            GlobalLogger.Error(F("HALOperationResult: "), HALOperationResultToString(readResult));
            GlobalLogger.setLastEntrySource(outDevice->GetType());
            /*message += "\"error\":\"";
            message += HALOperationResultToString(readResult);
            message += '"';
            message += ",\"target_type\":\"";
            message += device->GetType();
            message += '"';*/
            return false;
        }
        message += DeviceConstStrings::value;
        message += valueStr;
        return true;
    }

    bool CommandExecutor::execCmd(ZeroCopyString& zcStr, std::string& message) {
        ZeroCopyString zcPath = zcStr.SplitOffHead('/');
        // check if have uid given
        if (zcPath.IsEmpty()) {
            GlobalLogger.Error(F("uid path empty"));
            //message += "\"error\":\"uid path is empty\"";
            return false;
        }
        // first check if device exists
        UIDPath uidPath(zcPath);
        Device* outDevice = nullptr;
        DeviceFindResult devFindRes = Manager::findDevice(uidPath, outDevice);
        if (devFindRes != DeviceFindResult::Success) {
            GlobalLogger.Error(F("device not found: "), zcPath);
            GlobalLogger.setLastEntrySource(DeviceFindResultToString(devFindRes));
            //message += "\"error\":\"device not found\"";
            //message += ",\"uidpath\":\"" + zcPath.ToString() + "\"";
            return false;
        }

        HALOperationResult res = HALOperationResult::NotSet;
        if (zcStr.NotEmpty()) {
            //UIDPath path(zcPath); // obsolete
            res = outDevice->exec(zcStr);
        } else {
            //UIDPath path(zcStr); // obsolete
            res = outDevice->exec();
        }
        if (res != HALOperationResult::Success) {
            GlobalLogger.Error(F("HALOperationResult: "), HALOperationResultToString(res));
            GlobalLogger.setLastEntrySource(outDevice->GetType());
            /*message += "\"error\":\"";
            message += HALOperationResultToString(res);
            message += '"';
            message += ",\"target_type\":\"";
            message += device->GetType();
            message += '"';*/
            return false;
        }
        return true;
    }

}