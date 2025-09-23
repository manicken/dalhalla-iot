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

#include "CharArrayHelpers.h"

namespace CharArray {
    

    uint32_t countChar(const char* str, char ch) {
        uint32_t count = 0;
        while (*str) {
            if (*str == ch) count++;
            str++;
        }
        return count;
    }

    const uint32_t* getIndicies(const char* str, char ch, uint32_t& outCount) {
        outCount = countChar(str, ch);
        if (outCount == 0) return nullptr;
        uint32_t* indicies = new uint32_t[outCount];
        uint32_t index = 0;
        uint32_t strIndex = 0;
        while (*str) {
            if (*str == ch) { 
                indicies[index++] = strIndex;
            }
            str++;
            strIndex++;
        }
        return indicies;
    }

    const char** getPointers(const char* str, char ch, uint32_t& outCount) {
        outCount = countChar(str, ch);
        if (outCount == 0) return nullptr;
        const char** pointers = new const char*[outCount];
        uint32_t index = 0;
        while (*str) {
            if (*str == ch) { 
                pointers[index++] = str;
            }
            str++;
            //strIndex++;
        }
        return pointers;
    }

    bool equalsIgnoreCase(const char* a, const char* b) {
        while (*a && *b) {
            if (std::tolower(*a) != std::tolower(*b)) {
                return false;
            }
            ++a;
            ++b;
        }
        return *a == *b; // Ensure both strings ended
    }

    // Counts tokens in the input string without modifying the original
    int count_tokens(const char* str, const char* delimiters) {
        int count = 0;
        bool inToken = false;
        while (*str) {
            if (strchr(delimiters, *str)) {
                inToken = false;
            } else if (!inToken) {
                inToken = true;
                ++count;
            }
            ++str;
        }
        return count;
    }

    // Simplified pseudo-implementation
    //static char *current_position = NULL;

    char *my_strtok(char **str, const char *delimiters) {
        if (*str == NULL) {
            return NULL;
        }

        // Skip leading delimiters
        *str += strspn(*str, delimiters);
        if (**str == '\0') {
            return NULL;
        }

        char *token_start = *str;

        // Find end of token
        *str += strcspn(*str, delimiters);

        if (**str != '\0') {
            **str = '\0';   // Null-terminate token
            (*str)++;       // Advance past null terminator
        } else {
            *str = NULL;    // End of string reached
        }

        return token_start;
    }

    bool EndsWith(const char* str, const char* suffix) {
        if (!str || !suffix) return false;

        size_t str_len = strlen(str);
        size_t suffix_len = strlen(suffix);

        if (suffix_len > str_len) return false;

        return strcmp(str + (str_len - suffix_len), suffix) == 0;
    }

    bool StrEqualsICAny(const char* text, const char* const* candidates) {
            for (int i = 0; candidates[i] != nullptr; ++i) {
                if (StrEqualsIC(text, candidates[i])) {
                    return true;
                }
            }
            return false;
        }

/*
    StringView::StringView() : ptr(nullptr), len(0) {}
    StringView::StringView(const char* p, size_t l) : ptr(p), len(l) {}
    bool StringView::empty() const { return len == 0; }
    std::string StringView::toString() const { return std::string(ptr, len); } // for debug/logging
    */
}