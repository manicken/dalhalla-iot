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

#include "HAL_JSON_SCRIPT_ENGINE_Parser_Token.h"

namespace HAL_JSON {
    namespace ScriptEngine {

        void Tokens::ReportError(const char* msg) {
    #if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
            std::cout << "Error: " << msg << std::endl;
    #else
            GlobalLogger.Error(F("Rule Set Parse:"), msg);
    #endif
        }
        TokenType GetFundamentalTokenType(ZeroCopyString& zcStrType) {
            int zcStrLength = zcStrType.Length();
#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
            //printf("\nGetFundamentalTokenType: >>>%s<<< (%d)\n", zcStrType.ToString().c_str(),zcStrLength);
#endif
            // using a length based lockup for faster compare/lockup
            switch (zcStrLength) {
                case 1:  // ";" or "\"
                    if (zcStrType.EqualsIC(";")) return TokenType::ActionSeparator;
                    if (zcStrType.EqualsIC("\\")) return TokenType::ActionJoiner;
                    break;

                case 2:  // "if", "do", "on", "or"
                    if (zcStrType.EqualsIC("if")) return TokenType::If;
                    if (zcStrType.EqualsIC("do")) return TokenType::Then;
                    if (zcStrType.EqualsIC("on")) return TokenType::On;
                    if (zcStrType.EqualsIC("or")) return TokenType::Or;
                    break;

                case 3:  // "and"
                    if (zcStrType.EqualsIC("and")) return TokenType::And;
                    break;

                case 4:  // "else", "then"
                    if (zcStrType.EqualsIC("else")) return TokenType::Else;
                    if (zcStrType.EqualsIC("then")) return TokenType::Then;
                    break;

                case 5:  // "endif", "endon"
                    if (zcStrType.EqualsIC("endif")) return TokenType::EndIf;
                    if (zcStrType.EqualsIC("endon"))  return TokenType::EndOn;
                    break;

                case 6:  // "elseif"
                    if (zcStrType.EqualsIC("elseif")) return TokenType::ElseIf;
                    break;
                default:
                    return TokenType::NotSet;
            }
            return TokenType::NotSet;
        }
/*
        TokenType GetFundamentalTokenType(ZeroCopyString& zcStrType) {
        #if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
            //std::cout << "GetFundamentalTokenType: >>>" << str << "<<<\n";
        #endif
            if (zcStrType.EqualsIC("if")) return TokenType::If;
            else if (zcStrType.EqualsIC("endif")) return TokenType::EndIf;
            else if (zcStrType.EqualsIC("else")) return TokenType::Else;
            else if (zcStrType.EqualsIC("elseif")) return TokenType::ElseIf;
            else if (zcStrType.EqualsIC("then")) return TokenType::Then;
            else if (zcStrType.EqualsIC("do")) return TokenType::Then;
            else if (zcStrType.EqualsIC("on")) return TokenType::On;
            else if (zcStrType.EqualsIC("and")) return TokenType::And;
            else if (zcStrType.EqualsIC("or")) return TokenType::Or;
            else if (zcStrType.EqualsIC(";")) return TokenType::ActionSeparator;
            else if (zcStrType.EqualsIC("\\")) return TokenType::ActionJoiner;
            else if (zcStrType.EqualsIC("endon")) return TokenType::EndOn;
            else return TokenType::NotSet;
        }
*/
        const char* TokenTypeToString(TokenType type) {
            switch (type) {
                case TokenType::NotSet: return "NotSet";
                case TokenType::On: return "On";
                case TokenType::EndOn: return "EndOn";
                case TokenType::If: return "If";
                case TokenType::EndIf: return "EndIf";
                case TokenType::IfCondition: return "IfCondition";
                case TokenType::Else: return "Else";
                case TokenType::ElseIf: return "ElseIf";
                case TokenType::Then: return "Then";
                case TokenType::And: return "And";
                case TokenType::Or: return "Or";
                case TokenType::ActionSeparator: return "ActionSeparator";
                case TokenType::ActionJoiner: return "ActionJoiner";
                case TokenType::Action: return "Action";
                case TokenType::Merged: return "Merged";
                case TokenType::Ignore: return "Ignore";
                default: return "Unknown";
            }
        }

        Token::Token(): type(TokenType::NotSet) {
            Set(nullptr, nullptr, -1, -1);
        }

        void Token::Set(const char* _start, const char* _end, int _line, int _column) {
            start = _start;
            end = _end;
            line = _line;
            column = _column;
            itemsInBlock = 0;
            hasElse = 0;
            if (start != nullptr && end != nullptr) { // start is only nullptr when the token array is first created
                ZeroCopyString zcStrType(start, end);
                type = GetFundamentalTokenType(zcStrType);
                //printf("\nFundamentalTokenType is set to: %s from %s (%d)\n", TokenTypeToString(type), zcStrType.ToString().c_str(), zcStrType.Length());
            }
        }
        Token::~Token() {
            // nothing to do here as no dynamic memory is allocated
        }

        void Token::MarkTokenGroup(int size, TokenType constructType) {
            if (size == 1) {
                this->type = constructType;
                itemsInBlock = 1; // was = 0 before
                return;
            }
            itemsInBlock = size;
            
            // mark the following and inclusive this tokens merged
            this[0].type = constructType;
            for (int i = 1; i < size; i++) {

                if (constructType == TokenType::Action) {
                    if (this[i].type == TokenType::ActionJoiner) {
                        this[i].type = TokenType::Ignore;
                        continue;
                    }

                }
                this[i].type = TokenType::Merged;
            }
        }

        bool Token::AnyType(const TokenType* candidates) {
            while(*candidates != TokenType::NotSet) {
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

        Token& Tokens::GetNextAndConsume() {
            if (currIndex >= count) {
                printf("\n !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! token GetNextAndConsume() EXTREME ERROR out of bounds\n");
                static Token EmptyToken;
                return EmptyToken;
            }
            return items[currIndex++];
        }

        Token& Tokens::Current() {
            if (currIndex >= count) {
                printf("\n !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! token Current() EXTREME ERROR out of bounds\n");
                static Token EmptyToken;
                return EmptyToken;
            }
            return items[currIndex];
        }
        const Token& Tokens::Current() const {
            if (currIndex >= count) {
                printf("\n !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! token Current() const EXTREME ERROR out of bounds\n");
                static Token EmptyToken;
                return EmptyToken;
            }
            return items[currIndex];
        }

        bool Tokens::SkipIgnoresAndEndIf() {
            while (currIndex < count) {
                Token& token = Current();
                if (token.type != TokenType::Ignore && token.type != TokenType::EndIf)
                    break;
                ReportTokenInfo(token, "--------- skipping token:");
                currIndex++;
            }

            if (currIndex >= count) {
                ReportError("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Unexpected end of tokens while building items");
                return false;
            }
            return true;
        }

        Tokens::Tokens() : zeroCopy(true), firstTokenStartOffset(nullptr), items(nullptr), count(0), rootBlockCount(0), currentEndIndex(0), currIndex(0) {}

        Tokens::Tokens(int count) : zeroCopy(false), firstTokenStartOffset(nullptr), items(new Token[count]), count(count), rootBlockCount(0), currentEndIndex(count), currIndex(0) { }
        
        Tokens::~Tokens() {
            if (zeroCopy == false)
                delete[] items;
        }
        std::string Tokens::ToString() {
            std::string str;
            //str += "firstTokenStartOffset:"; str += (firstTokenStartOffset?"true ":"false ");
            for (int i=0;i<count;i++) {
                ZeroCopyString zcStrCopy = items[i];
                //if (i==0 && firstTokenStartOffset != nullptr) zcStrCopy.start = firstTokenStartOffset;
                str += zcStrCopy.ToString();
            }
            return str;
        }
        std::string Tokens::SliceToString() {
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


        void ReportTokenInfo(const Token& t, const char* msg, const char* param) {
            std::string message = " (line " + std::to_string(t.line) + ", col " + std::to_string(t.column) + "): " + msg;
            if (param != nullptr)
                message += param;
    #if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
            std::cout << "Info " << message << std::endl;
    #else
            //GlobalLogger.Info(F("Token:"), message.c_str());
    #endif
        }
        void ReportTokenError(const Token& t, const char* msg, const char* param) {
            std::string message = " (line " + std::to_string(t.line) + ", col " + std::to_string(t.column) + "): " + msg;
            if (param != nullptr)
                message += param;
    #if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
            std::cerr << "Error " << message << std::endl;
    #else
            GlobalLogger.Error(F("Token:"), message.c_str());
    #endif
        }
        void ReportTokenWarning(const Token& t, const char* msg, const char* param) {
            std::string message = " (line " + std::to_string(t.line) + ", col " + std::to_string(t.column) + "): " + msg;
            if (param != nullptr)
                message += param;
    #if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
            std::cout << "Warning " << message << std::endl;
    #else
            GlobalLogger.Warn(F("Token:"), message.c_str());
    #endif
        }

        std::string PrintTokens(Tokens& _tokens, int subTokenIndexOffset) {
            Token* tokens = _tokens.items;
            int tokenCount = _tokens.count;
            std::string msg;
            if (subTokenIndexOffset == 0)
                msg += "rootLevelBlockCount: " + std::to_string(_tokens.rootBlockCount) + "\n";

            if (_tokens.firstTokenStartOffset != nullptr) {
                msg += "firstTokenStartOffset set, ";
            }
            bool lastWasBlock = false;
            for (int i=0;i<tokenCount;i++) {
                Token& tok = tokens[i];
                std::string msgLine;
                if (subTokenIndexOffset > 0) msgLine += "  ";
                // skip duplicate prints as theese will be printed as the Tokens block
                // ignore types are allways part of a token block 
                if ((tok.type == TokenType::Merged) && subTokenIndexOffset == 0) {
                    //msgLine += " skipping: ";
                    continue; 
                }
                
                
                
                // only print one level down and only if the type is Action or IfCondition
                if (subTokenIndexOffset == 0 && tok.itemsInBlock > 1 && (tok.type == TokenType::Action || tok.type == TokenType::IfCondition)) {
                    if (lastWasBlock == false)
                        msgLine += "\n";
                    msgLine += "Tokens block:\n";
                    Tokens tokens;
                    tokens.items = &tok;
                    tokens.count = tok.itemsInBlock;
                    msgLine += PrintTokens(tokens, i);
                    lastWasBlock = true;
                } else {
                    msgLine +=
                    "Token(" + std::to_string(i+subTokenIndexOffset) + "): " +
                    "(line:" + std::to_string(tok.line) + 
                    ", col:" + std::to_string(tok.column) + 
                    ", count:" + std::to_string(tok.itemsInBlock) + 
                    ", type:" + TokenTypeToString(tok.type) +
                    ((tok.type == TokenType::If)?((tok.hasElse==1)?", hasElse:true":", hasElse:false"):"")+
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