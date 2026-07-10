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

#pragma once

#include <Arduino.h>
#include <stdlib.h>
#include <DALHAL/Core/Types/DALHAL_ZeroCopyString.h>
#include <queue>
#include <functional>
#if defined(_WIN32) || defined(__linux__) || defined(__MAC__)
#include <mutex>
#endif


//#define DALHAL_CommandExecutor_DEBUG_CMD

#include <DALHAL/Core/Types/DALHAL_OperationResult.h>
#include <DALHAL/API/DALHAL_CommandCallback.h>
#include <DALHAL/API/DALHAL_StringBuilderStreamer.h>
#include <DALHAL/Core/Types/DALHAL_ConstExpressionConstStrings.h>

namespace DALHAL {

    struct PendingRequest {
        std::string command;
        CommandCallback cb;
    };

    struct CommandNode {
        using FlagsType = uint8_t;
        struct Flags {
            static constexpr FlagsType HIDDEN = 0x01;
            static constexpr FlagsType AUTOGEN_BUTTON = 0x02;
            static constexpr FlagsType DANGER = 0x04;
        };
        ConstExpressionStringComparableFn name;
        ConstExpressionStringFn help;
        FlagsType flags;

        using Execute = HALOperationResult (*)(ZeroCopyString& args, CommandCallback cb);
        
        Execute execute;

        const CommandNode* children;
        const size_t children_count;

        constexpr CommandNode(ConstExpressionStringComparableFn name, Execute execute, const CommandNode* children, const size_t children_count, ConstExpressionStringFn help) 
            : name(name), help(help), flags(0), execute(execute), children(children), children_count(children_count) {}

        constexpr CommandNode(ConstExpressionStringComparableFn name, const CommandNode* children, const size_t children_count, ConstExpressionStringFn help) 
            : name(name), help(help), flags(0), execute(nullptr), children(children), children_count(children_count) {}

        constexpr CommandNode(ConstExpressionStringComparableFn name, Execute execute, ConstExpressionStringFn help) 
            : name(name), help(help), flags(0), execute(execute), children(nullptr), children_count(0) {}

        // with flags
        constexpr CommandNode(ConstExpressionStringComparableFn name, Execute execute, const CommandNode* children, const size_t children_count, FlagsType flags, ConstExpressionStringFn help) 
            : name(name), help(help), flags(flags), execute(execute), children(children), children_count(children_count) {}

        constexpr CommandNode(ConstExpressionStringComparableFn name, const CommandNode* children, const size_t children_count, FlagsType flags, ConstExpressionStringFn help) 
            : name(name), help(help), flags(flags), execute(nullptr), children(children), children_count(children_count) {}

        constexpr CommandNode(ConstExpressionStringComparableFn name, Execute execute, FlagsType flags, ConstExpressionStringFn help) 
            : name(name), help(help), flags(flags), execute(execute), children(nullptr), children_count(0) {}
    };

    class CommandExecutor {
    public:

        static std::queue<PendingRequest> g_pending;

        
        

#if defined(ESP32)
  static portMUX_TYPE g_pendingMux;
  #define CommandExecutor_LOCK_QUEUE()   portENTER_CRITICAL(&DALHAL::CommandExecutor::g_pendingMux)
  #define CommandExecutor_UNLOCK_QUEUE() portEXIT_CRITICAL(&DALHAL::CommandExecutor::g_pendingMux)
#elif defined(_WIN32) || defined(__linux__) || defined(__MAC__)
  static std::mutex g_pendingMutex;
  #define CommandExecutor_LOCK_QUEUE()   DALHAL::CommandExecutor::g_pendingMutex.lock()
  #define CommandExecutor_UNLOCK_QUEUE() DALHAL::CommandExecutor::g_pendingMutex.unlock()
#elif defined(ESP8266)
  //static portMUX_TYPE g_pendingMux; not used on esp8266
  //#define CommandExecutor_LOCK_QUEUE()   noInterrupts()
  //#define CommandExecutor_UNLOCK_QUEUE() interrupts()
  #define CommandExecutor_LOCK_QUEUE()   ETS_INTR_LOCK()
  #define CommandExecutor_UNLOCK_QUEUE() ETS_INTR_UNLOCK()
#endif

        /** 
         * having ZeroCopyString as writable ref, 
         * as that would be useful when passing it to other parsing functions
         */
        static bool execute(ZeroCopyString& zcStr, CommandCallback cb);

        struct ReadWriteCmdParameters {
            bool isHelpRequest;
            bool cmdIsPresent;
            bool isBracketOp;
            ZeroCopyString zcUid;
            ZeroCopyString zcBracketParameter;
            ZeroCopyString zcCmd;
            ZeroCopyString zcParameters;
            ReadWriteCmdParameters(ZeroCopyString& zcStr);

        };

        struct ExecCmdParameters {
            bool isHelpRequest;
            bool cmdIsPresent;
            ZeroCopyString zcUid;
            ZeroCopyString zcCmd;
            ExecCmdParameters(ZeroCopyString& zcStr);

        };
        
    };
}