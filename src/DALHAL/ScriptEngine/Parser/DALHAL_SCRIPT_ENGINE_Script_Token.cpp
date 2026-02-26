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

#include "DALHAL_SCRIPT_ENGINE_Script_Token.h"

#include "../../Support/DALHAL_Logger.h"

namespace DALHAL {
    namespace ScriptEngine {

        void ScriptTokens::ReportError(const char* msg) {
    #if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
            std::cout << "Error: " << msg << std::endl;
    #else
            GlobalLogger.Error(F("Rule Set Parse:"), msg);
    #endif
        }
        ScriptTokenType GetFundamentalScriptTokenType(ZeroCopyString& zcStrType) {
            int zcStrLength = zcStrType.Length();
#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
            //printf("\nGetFundamentalTokenType: >>>%s<<< (%d)\n", zcStrType.ToString().c_str(),zcStrLength);
#endif
            // using a length based lockup for faster compare/lockup
            switch (zcStrLength) {
                case 1:  // ";" or "\"
                    if (zcStrType.EqualsIC(";")) return ScriptTokenType::ActionSeparator;
                    if (zcStrType.EqualsIC("\\")) return ScriptTokenType::ActionJoiner;
                    break;

                case 2:  // "if", "do", "on", "or"
                    if (zcStrType.EqualsIC("if")) return ScriptTokenType::If;
                    if (zcStrType.EqualsIC("do")) return ScriptTokenType::Then;
                    if (zcStrType.EqualsIC("on")) return ScriptTokenType::On;
                    if (zcStrType.EqualsIC("or")) return ScriptTokenType::Or;
                    break;

                case 3:  // "and"
                    if (zcStrType.EqualsIC("and")) return ScriptTokenType::And;
                    break;

                case 4:  // "else", "then"
                    if (zcStrType.EqualsIC("else")) return ScriptTokenType::Else;
                    if (zcStrType.EqualsIC("then")) return ScriptTokenType::Then;
                    break;

                case 5:  // "endif", "endon"
                    if (zcStrType.EqualsIC("endif")) return ScriptTokenType::EndIf;
                    if (zcStrType.EqualsIC("endon"))  return ScriptTokenType::EndOn;
                    break;

                case 6:  // "elseif"
                    if (zcStrType.EqualsIC("elseif")) return ScriptTokenType::ElseIf;
                    break;
                default:
                    return ScriptTokenType::NotSet;
            }
            return ScriptTokenType::NotSet;
        }

        const char* ScriptTokenTypeToString(ScriptTokenType type) {
            switch (type) {
                case ScriptTokenType::NotSet: return "NotSet";
                case ScriptTokenType::On: return "On";
                case ScriptTokenType::EndOn: return "EndOn";
                case ScriptTokenType::If: return "If";
                case ScriptTokenType::EndIf: return "EndIf";
                case ScriptTokenType::IfCondition: return "IfCondition";
                case ScriptTokenType::Else: return "Else";
                case ScriptTokenType::ElseIf: return "ElseIf";
                case ScriptTokenType::Then: return "Then";
                case ScriptTokenType::And: return "And";
                case ScriptTokenType::Or: return "Or";
                case ScriptTokenType::ActionSeparator: return "ActionSeparator";
                case ScriptTokenType::ActionJoiner: return "ActionJoiner";
                case ScriptTokenType::Action: return "Action";
                case ScriptTokenType::Merged: return "Merged";
                case ScriptTokenType::Ignore: return "Ignore";
                default: return "Unknown";
            }
        }

        ScriptToken::ScriptToken(): type(ScriptTokenType::NotSet) {
            Set(nullptr, nullptr, -1, -1);
        }

        void ScriptToken::Set(const char* _start, const char* _end, int _line, int _column) {
            start = _start;
            end = _end;
            line = _line;
            column = _column;
            itemsInBlock = 0;
            hasElse = 0;
            if (start != nullptr && end != nullptr) { // start is only nullptr when the token array is first created
                ZeroCopyString zcStrType(start, end);
                type = GetFundamentalScriptTokenType(zcStrType);
                // the problem with the following is that
                // the rest of the parser is setting it to merged/action 
                // so the string literal type is lost
                // so keeping the " is a good way to check if it's a string literal
                /*if (type == TokenType::StringLiteral) { 
                    start++; //  remove starting "
                    end--; // remove ending "
                }*/
                //printf("\nFundamentalTokenType is set to: %s from %s (%d)\n", TokenTypeToString(type), zcStrType.ToString().c_str(), zcStrType.Length());
            }
        }
        ScriptToken::~ScriptToken() {
            // nothing to do here as no dynamic memory is allocated
        }

        void ScriptToken::MarkTokenGroup(int size, ScriptTokenType constructType) {
            if (size == 1) {
                this->type = constructType;
                itemsInBlock = 1; // was = 0 before
                return;
            }
            itemsInBlock = size;
            
            // mark the following and inclusive this tokens merged
            this[0].type = constructType;
            for (int i = 1; i < size; i++) {

                if (constructType == ScriptTokenType::Action) {
                    if (this[i].type == ScriptTokenType::ActionJoiner) {
                        this[i].type = ScriptTokenType::Ignore;
                        continue;
                    }

                }
                this[i].type = ScriptTokenType::Merged;
            }
        }

        bool ScriptToken::AnyType(const ScriptTokenType* candidates) {
            while(*candidates != ScriptTokenType::NotSet) {
                if (type == *candidates) return true;
                candidates++;
            }
            return false;
        }

        //    ████████  ██████  ██   ██ ███████ ███    ██ ███████ 
        //       ██    ██    ██ ██  ██  ██      ████   ██ ██      
        //       ██    ██    ██ █████   █████   ██ ██  ██ ███████ 
        //       ██    ██    ██ ██  ██  ██      ██  ██ ██      ██ 
        //       ██     ██████  ██   ██ ███████ ██   ████ ███████ 

        ScriptToken& ScriptTokens::GetNextAndConsume() {
            if (currIndex >= count) {
                printf("\n !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! token GetNextAndConsume() EXTREME ERROR out of bounds\n");
                static ScriptToken EmptyToken;
                return EmptyToken;
            }
            return items[currIndex++];
        }

        ScriptToken& ScriptTokens::Current() {
            if (currIndex >= count) {
                printf("\n !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! token Current() EXTREME ERROR out of bounds\n");
                static ScriptToken EmptyToken;
                return EmptyToken;
            }
            return items[currIndex];
        }
        const ScriptToken& ScriptTokens::Current() const {
            if (currIndex >= count) {
                printf("\n !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! token Current() const EXTREME ERROR out of bounds\n");
                static ScriptToken EmptyToken;
                return EmptyToken;
            }
            return items[currIndex];
        }

        bool ScriptTokens::SkipIgnoresAndEndIf() {
            while (currIndex < count) {
                ScriptToken& token = Current();
                if (token.type != ScriptTokenType::Ignore && token.type != ScriptTokenType::EndIf)
                    break;
                token.ReportTokenInfo("--------- skipping token:");
                currIndex++;
            }

            if (currIndex >= count) {
                ReportError("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Unexpected end of tokens while building items");
                return false;
            }
            return true;
        }

        ScriptTokens::ScriptTokens() : zeroCopy(true), firstTokenStartOffset(nullptr), items(nullptr), count(0), rootBlockCount(0), currentEndIndex(0), currIndex(0) {}

        ScriptTokens::ScriptTokens(int count) : zeroCopy(false), firstTokenStartOffset(nullptr), items(new ScriptToken[count]), count(count), rootBlockCount(0), currentEndIndex(count), currIndex(0) { }
        
        ScriptTokens::~ScriptTokens() {
            if (zeroCopy == false)
                delete[] items;
        }
        std::string ScriptTokens::ToString() {
            std::string str;
            //str += "firstTokenStartOffset:"; str += (firstTokenStartOffset?"true ":"false ");
            for (int i=0;i<count;i++) {
                ZeroCopyString zcStrCopy = items[i];
                //if (i==0 && firstTokenStartOffset != nullptr) zcStrCopy.start = firstTokenStartOffset;
                str += zcStrCopy.ToString();
            }
            return str;
        }
        int ScriptTokens::SliceTokenCount() {
            return currentEndIndex - currIndex;
        }
        std::string ScriptTokens::SliceToString() {
            std::string str;

            if (!items) return str;

            // clamp indices to valid range
            int start = currIndex;
            if (start < 0) start = 0;
            if (start >= count) return str;

            int end = currentEndIndex;
            if (end < start) return str;
            if (end > count) end = count;

            for (int i = start; i < end; ++i) {
                ZeroCopyString zcStrCopy = items[i];

                // the FIRST token *in this slice* may have a custom start offset
                if (i == start && firstTokenStartOffset != nullptr) {
                    // only adjust if pointer is inside token range
                    if (zcStrCopy.start <= firstTokenStartOffset && firstTokenStartOffset <= zcStrCopy.end) {
                        zcStrCopy.start = firstTokenStartOffset;
                    }
                }

                // ToString already guards invalid start/end
                std::string piece = zcStrCopy.ToString();
                if (!piece.empty()) {
                    str += piece;
                }
            }
            return str;
        }

        std::string PrintScriptTokens(ScriptTokens& _tokens, int subTokenIndexOffset) {
            ScriptToken* tokens = _tokens.items;
            int tokenCount = _tokens.count;
            std::string msg;
            if (subTokenIndexOffset == 0)
                msg += "rootLevelBlockCount: " + std::to_string(_tokens.rootBlockCount) + "\n";

            if (_tokens.firstTokenStartOffset != nullptr) {
                msg += "firstTokenStartOffset set, ";
            }
            bool lastWasBlock = false;
            for (int i=0;i<tokenCount;i++) {
                ScriptToken& tok = tokens[i];
                std::string msgLine;
                if (subTokenIndexOffset > 0) msgLine += "  ";
                // skip duplicate prints as theese will be printed as the Tokens block
                // ignore types are allways part of a token block 
                if ((tok.type == ScriptTokenType::Merged) && subTokenIndexOffset == 0) {
                    //msgLine += " skipping: ";
                    continue; 
                }
                
                // only print one level down and only if the type is Action or IfCondition
                if (subTokenIndexOffset == 0 && tok.itemsInBlock > 1 && (tok.type == ScriptTokenType::Action || tok.type == ScriptTokenType::IfCondition)) {
                    if (lastWasBlock == false)
                        msgLine += "\n";
                    msgLine += "Tokens block:\n";
                    ScriptTokens tokens;
                    tokens.items = &tok;
                    tokens.count = tok.itemsInBlock;
                    msgLine += PrintScriptTokens(tokens, i);
                    lastWasBlock = true;
                } else {
                    msgLine +=
                    "Token(" + std::to_string(i+subTokenIndexOffset) + "): " +
                    "(line:" + std::to_string(tok.line) + 
                    ", col:" + std::to_string(tok.column) + 
                    ", count:" + std::to_string(tok.itemsInBlock) + 
                    ", type:" + ScriptTokenTypeToString(tok.type) +
                    ((tok.type == ScriptTokenType::If)?((tok.hasElse==1)?", hasElse:true":", hasElse:false"):"")+
                    "): ";

                    lastWasBlock = false;
                    //msgLine += " >>> " + tok.ToString() + " <<<";// size: " + std::to_string(tok.Length());// std::string(tok.text);
                    msgLine += tok.ToString();
                }
                msg += msgLine + "\n";
            }
            return msg;
        }

    }
}