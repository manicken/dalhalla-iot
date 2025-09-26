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

#include <Arduino.h> // Needed for String class
#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
#include <iostream> // including this take 209512 bytes flash

#endif
#include <string>
#include <cstdint>


#include <vector>
#include <stack>
#include "HAL_JSON_SCRIPT_ENGINE_Parser_Token.h"
#include "HAL_JSON_SCRIPT_ENGINE_Expression_Token.h"
#include "HAL_JSON_SCRIPT_ENGINE_Expression_Parser.h"

#include "../../Support/ConvertHelper.h"
#include "../../Support/CharArrayHelpers.h"
#include "../HAL_JSON_ZeroCopyString.h"


#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
#include <chrono>
#include <iostream>

#define MEASURE_TIME(message, block) \
do { \
    auto _mt_start = std::chrono::high_resolution_clock::now(); \
    block; \
    auto _mt_end = std::chrono::high_resolution_clock::now(); \
    std::chrono::duration<double, std::milli> _mt_duration = _mt_end - _mt_start; \
    std::cout << message << _mt_duration.count() << " ms\n"; \
} while (0)

#else
// On embedded builds: expands to nothing, zero overhead
#define MEASURE_TIME(message, block) do { block; } while (0)
#endif



namespace HAL_JSON {
    namespace ScriptEngine {
        
        enum class ParserError {
            FileNotFound,
            FileEmpty,
            FileContentsAllocFail
        };

        struct AssignmentParts {
            Token lhs;
            char op;      // assignment operators first char is enough (e.g. "=", "+=", "<<=")
            Tokens rhs;

            inline void Clear() {
                lhs = {};
                rhs.count = 0;
                rhs.items = nullptr;
                rhs.currIndex = 0;
                rhs.firstTokenStartOffset = nullptr;
                rhs.rootBlockCount = 0;
            }
        };

        class Parser {
        private:
            
            static void ReportError(const char* msg);
//#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
            static void ReportInfo(std::string msg);
//#endif
            //static inline bool IsType(const Token& t, const char* str) { return t.EqualsIC(str); }

            /** special note,
              *  this function do not remove additional \r characters in \r\n \n\r 
              *  it just replaces them with spaces for easier parsing 
              */
            static void FixNewLines(char* buffer);
            static void StripComments(char* buffer);

            
            /** used only for scripts */
            static bool Tokenize(char* buffer, Tokens& tokens);
            

            /** used by VerifyBlocks */
            static int Count_IfTokens(Tokens& tokens);

            /** verify if/on blocks so that they follow the structure
             * on/if <trigger>/<condition> do/then <action(s)> endon/endif
             * it also verify that on blocks do only contain if blocks
             */
            static bool VerifyBlocks(Tokens& tokens);
            static int CountConditionTokens(Tokens& tokens, int start);

            /** 
             * merge Conditions into one token for easier parse,
             * if a AND/OR token is found they are 
             * replaced by && and || respective 
             */
            static bool MergeConditions(Tokens& tokens);

            /** merge actions so that each action 'line'
             *  is in one token for easier parse 
             *  this is a variant to MergeActions but
             *  allows the use of \ to make multiline spanning actions
             */
            static bool MergeActions2(Tokens& tokens);

            /** this is used together with EnsureActionBlocksContainItems */
            static void CountBlockItems(Tokens& tokens);
            static bool EnsureActionBlocksContainItems(Tokens& tokens);

            static bool VerifyConditionBlocks(Tokens& tokens);
            static bool VerifyActionBlocks(Tokens& tokens);
            static bool ValidateParseScript(Tokens& tokens, bool validateOnly);
            
        public:
        /** 
         * if the callback is set this is considered a Load function
         * if the callback is not set (nullptr) then it's validate only
         */
            static int CountTokens(char* buffer);
            /** can be used to tokenize a simple string, based on whitespace */
            static bool Tokenize(char* buffer, ZeroCopyString* items, int tokenCount);
            static bool ReadAndParseScriptFile(const char* filePath, void (*parsedOKcallback)(Tokens& tokens) = nullptr);


            static bool ParseExpressionTest(const char* filePath);
            static bool ParseActionExpressionTest(const char* filePath);

            static AssignmentParts* ExtractAssignmentParts(Tokens& _tokens);
        };
    }
}