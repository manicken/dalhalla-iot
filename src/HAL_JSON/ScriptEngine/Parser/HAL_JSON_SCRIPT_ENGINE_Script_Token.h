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
#include <chrono>
#endif
#include <string>
#include <cstdint>

#include <vector>
#include <stack>

#include "../../../Support/Logger.h"
#include "../../../Support/ConvertHelper.h"
#include "../../../Support/CharArrayHelpers.h"
#include "../../HAL_JSON_ZeroCopyString.h"
#include "HAL_JSON_SCRIPT_ENGINE_Token.h"

namespace HAL_JSON {
    namespace ScriptEngine {
        /** 
         * for the fundamentala types
         * this is set in the tokenize function.
         */
        enum class ScriptTokenType : uint16_t {
            /** used to make it easier to see unset tokens, is also used as terminator item when defining a list of token types */
            NotSet, 
            On,
            EndOn,
            If,
            EndIf,
            /** Do/Then token */
            Then,
            Else,
            ElseIf,
            IfCondition,
            /** used to mark actions, note if a action spans multiple tokens only the root is marked, the rest is marked Merged */
            Action,
            Merged,
            /** used both as a conditional and / && and as a action separator in one line scripts */
            And,
            /** used only as conditional || / or */
            Or,
            /** specific action separator currently defined as ; should be marked as Ignored When consumed*/
            ActionSeparator,
            /** used to split actions into multiple lines currently \ (note it need to have a space before and whitespace after), should be marked as Ignored When consumed*/
            ActionJoiner,
            /** marks the token to be ignored in futher parsing, should be marked as Ignored When consumed*/
            Ignore,
            
        };


        const char* ScriptTokenTypeToString(ScriptTokenType type);
        
        ScriptTokenType GetFundamentalScriptTokenType(ZeroCopyString& zcStrType);

        struct ScriptToken : public Token {
            using Token::Token;

            uint16_t itemsInBlock;
            uint16_t hasElse;
            ScriptTokenType type;

            ScriptToken();
            
            /* no copy/move constructors/assigments needed*/
            //Token(Token&) = delete;          // no copy constructor
            //Token& operator=(const Token&) = delete; // no copy assignment
            //Token(Token&& other) = delete;           // no move constructor
            //Token& operator=(Token&& other) = delete; // no move assignment

            void Set(const char* _start, const char* _end, int line, int column);
            /** 
             * used when 'grouping' tokens that actually belong to the same construct
             * note currently the valid types are only constructTypes:
             * TokenType::IfCondition
             * TokenType::Action
             */
            void MarkTokenGroup(int size, ScriptTokenType constructType);

            bool AnyType(const ScriptTokenType* candidates);
            ~ScriptToken();
        };
        /** Tokens are considered identical if their 'start' pointers are the same */
        inline bool operator==(const ScriptToken& lhs, const ScriptToken& rhs) { return lhs.start == rhs.start; }
        /** Tokens are considered not identical if their 'start' pointers are not the same */
        inline bool operator!=(const ScriptToken& lhs, const ScriptToken& rhs) { return lhs.start != rhs.start; }
        
        struct ScriptTokens {
        private:
            static void ReportError(const char* msg);
            bool zeroCopy;
        public:
            /** used when there is a situation where the first token need to be splitted
             * i.e. for example when parsing 
             * assigment action RHS expressions 
             * by the Expression_Parser such as
             * someVar =5 or someVar +=5
             * and also
             * someVar=5 or someVar+=5
             */
            const char* firstTokenStartOffset;
            ScriptToken* items;
            int count;
            /** root blocks count i.e. all on/if that is at root level only */
            int rootBlockCount;
            /** End position (exclusive). Valid range is [currIndex, currentEndIndex). */
            int currentEndIndex;
            /** 
             * Current token index during script parsing/loading.
             * Used to track the read position within the token list.
             */
            int currIndex;
            /** this will initialize this instance as a zeroCopyPointer storage */
            ScriptTokens();
            /** this will initialize this instance owned Token storage */
            ScriptTokens(int count);
            ~ScriptTokens();

            
            ScriptToken& Current();
            const ScriptToken& Current() const;
            ScriptToken& GetNextAndConsume();
            
            bool SkipIgnoresAndEndIf();

            // Optional: const version
            

            ScriptTokens(ScriptTokens&) = delete;          // no copy constructor
            ScriptTokens& operator=(const ScriptTokens&) = delete; // no copy assignment
            ScriptTokens(ScriptTokens&& other) = delete;           // no move constructor
            ScriptTokens& operator=(ScriptTokens&& other) = delete; // no move assignment
            std::string ToString();
            std::string SliceToString();
        };

        std::string PrintScriptTokens(ScriptTokens& tokens, int subTokenIndexOffset = 0);
        
    }
}