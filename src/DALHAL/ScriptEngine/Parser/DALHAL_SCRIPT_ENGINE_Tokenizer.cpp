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

#include "DALHAL_SCRIPT_ENGINE_Tokenizer.h"

#include "../../Core/Types/DALHAL_ZeroCopyString.h"
#include "DALHAL_SCRIPT_ENGINE_Token.h"
#include "DALHAL_SCRIPT_ENGINE_Script_Token.h"

namespace DALHAL {
    namespace ScriptEngine {
        inline void advance(char*& p, int& column) {
            p++;
            column++;
        }

        inline void advance(char*& p, int& line, int& column) {
            if (*p == '\n') { 
                line++;
                column = 1;
            } else {
                column++;
            }
            p++;
        }
        template <typename T>
        int ParseAndTokenize(char* buffer, T* tokens, int maxCount) {
            char* p = buffer;
            int tokenIndex = 0;
            int line = 1;
            int column = 1;

            while (*p) {
                // --- Skip whitespace and comments ---
                for (;;) {
                    if (*p == '\0') break;

                    // Whitespace
                    if (isspace(static_cast<unsigned char>(*p))) {
                        advance(p, line, column); // advance is newline-aware
                        continue;
                    }

                    // // single-line comment
                    if (p[0] == '/' && p[1] == '/') {
                        p+=2; // note here we don't touch column 
                        // Skip all characters until either the end of the string or a newline
                        while (*p && *p != '\n') { p++; }
                        continue;
                    }

                    // /* block comment */
                    if (p[0] == '/' && p[1] == '*') {
                        p += 2;
                        column += 2; // need to be aware as block comment can escape anywhere
                        while (*p) {
                            if (p[0] == '*' && p[1] == '/') {
                                p += 2;
                                column += 2;
                                break;
                            }
                            advance(p, line, column); // advance is newline-aware
                        }
                        continue;
                    }
                    // No more whitespace or comments
                    break;
                }

                if (!*p) break;

                // check for string literal start and handle it
                if (*p == '"') {
                    char* token_start = p;      // include the starting quote
                    int token_column = column;
                    int extraNewlines = 0;
                    // skip opening quote
                    advance(p, column);
                    
                    while (*p && *p != '"') {
                        if (*p == '\\' && *(p+1)) {
                            // skip '\'
                            advance(p, column);
                        } // skip escaped char or next non "
                        advance(p, extraNewlines, column); // advance is newline-aware
                    }

                    if (*p == '"') {
                        // skip closing quote
                        advance(p, column); 
                    }

                    // store string token
                    if (tokens) {
                        if (tokenIndex >= maxCount) return -1;
                        tokens[tokenIndex].Set(token_start, p, line, token_column);
                    }
                    line += extraNewlines;
                    tokenIndex++;
                    continue; // move to next token
                }

                // --- Start of token ---
                char* token_start = p;
                int token_column = column;

                // Find end of token
                while (*p) {
                    char c = *p;
                    // break on whitespace
                    if (isspace(static_cast<unsigned char>(c))) { break; }
                    // break on comment start inside token
                    if (c == '/' && (p[1] == '/' || p[1] == '*')) { break; }
                    // break if the character itself is a token separator (; or \)
                    if (c == ';' || c == '\\') break;

                    advance(p, column);
                }
                // --- Handle single-character token separators separately ---
                if (p[0] == ';' || p[0] == '\\') {
                    // If the token we just scanned has length > 0, store it first
                    if (token_start < p) {
                        if (tokens) {
                            if (tokenIndex >= maxCount) return -1;
                            tokens[tokenIndex].Set(token_start, p, line, token_column);
                        }
                        tokenIndex++;
                    }

                    // Then store the separator as its own token
                    if (tokens) {
                        if (tokenIndex >= maxCount) return -1;
                        tokens[tokenIndex].Set(p, p + 1, line, column);
                    }
                    tokenIndex++;

                    advance(p, column);
                    continue; // move to next token
                }

                // If the loop broke naturally (whitespace or comment), store the token
                if (tokens) {
                    if (tokenIndex >= maxCount) { return -1; } // error: mismatch, nearly impossible to happend
                    tokens[tokenIndex].Set(token_start, p, line, token_column);
                }
                tokenIndex++;
            }
            return tokenIndex; // count of tokens found
        }
        // Explicit instantiation for each type need to be after the template definition
        template int ParseAndTokenize<ZeroCopyString>(char* buffer, ZeroCopyString* tokens, int maxCount);
        template int ParseAndTokenize<Token>(char* buffer, Token* tokens, int maxCount);
        template int ParseAndTokenize<ScriptToken>(char* buffer, ScriptToken* tokens, int maxCount);
    }
}