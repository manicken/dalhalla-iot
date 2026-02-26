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

#include <Arduino.h>
#include <stdlib.h>
#include "../Core/Types/DALHAL_ZeroCopyString.h"
#include <queue>
#include <functional>
#if defined(_WIN32) || defined(__linux__) || defined(__MAC__)
#include <mutex>
#endif

#define DALHAL_CMD_EXEC_WRITE_CMD               "write"
#define DALHAL_CMD_EXEC_READ_CMD                "read"
#define DALHAL_CMD_EXEC_CMD                     "exec"
#define DALHAL_CMD_EXEC_RELOAD_CFG_JSON         "reloadcfg"
#define DALHAL_CMD_EXEC_RELOAD_CFG_JSON_SAFE    "reloadcfgsafe"
#define DALHAL_CMD_EXEC_RELOAD_SCRIPTS          "reloadscripts"
#define DALHAL_CMD_EXEC_PRINT_DEVICES           "printDevices"
#define DALHAL_CMD_EXEC_GET_AVAILABLE_GPIO_LIST "getAvailableGPIOs"
#define DALHAL_CMD_EXEC_PRINT_LOG_CONTENTS      "printlog"



#define DALHAL_CMD_EXEC_UINT32_TYPE        "uint32"
#define DALHAL_CMD_EXEC_BOOL_TYPE          "bool"
#define DALHAL_CMD_EXEC_FLOAT_TYPE         "float"
#define DALHAL_CMD_EXEC_JSON_STR_TYPE      "json"
#define DALHAL_CMD_EXEC_STRING_TYPE        "string"

//#define DALHAL_CommandExecutor_DEBUG_CMD

namespace DALHAL {
    using CommandCallback = std::function<void(const std::string& response)>;

    struct PendingRequest {
        std::string command;
        CommandCallback cb;
    };

    class CommandExecutor {
    public:

        static std::queue<PendingRequest> g_pending;
        

#if defined(ESP32)
  static portMUX_TYPE g_pendingMux;
  #define CommandExecutor_LOCK_QUEUE()   portENTER_CRITICAL(&CommandExecutor::g_pendingMux)
  #define CommandExecutor_UNLOCK_QUEUE() portEXIT_CRITICAL(&CommandExecutor::g_pendingMux)
#elif defined(_WIN32) || defined(__linux__) || defined(__MAC__)
  static std::mutex g_pendingMutex;
  #define CommandExecutor_LOCK_QUEUE()   CommandExecutor::g_pendingMutex.lock()
  #define CommandExecutor_UNLOCK_QUEUE() CommandExecutor::g_pendingMutex.unlock()
#elif defined(ESP8266)
  static portMUX_TYPE g_pendingMux;
  #define CommandExecutor_LOCK_QUEUE()   noInterrupts()
  #define CommandExecutor_UNLOCK_QUEUE() interrupts()
#endif

        struct Result {
            bool success = true;
            // must be a string as the ownership must be here
            std::string message;
        };
        
        /** 
         * having ZeroCopyString as writable ref, 
         * as that would be useful when passing it to other parsing functions
         */
        static bool execute(ZeroCopyString& zcStr, CommandCallback cb);
    private:
        struct ReadWriteCmdParameters {
            ZeroCopyString zcType;
            ZeroCopyString zcUid;
            ZeroCopyString zcValue;
            ReadWriteCmdParameters(ZeroCopyString& zcStr);
#ifdef DALHAL_CommandExecutor_DEBUG_CMD
            std::string ToString();
#endif
        };
        static bool reloadJSON(ZeroCopyString& zcStr, std::string& message);
        static bool writeCmd(ZeroCopyString& zcStr, std::string& message);
        static bool readCmd(ZeroCopyString& zcStr, std::string& message);
        static bool execCmd(ZeroCopyString& zcStr, std::string& message);
    };
}