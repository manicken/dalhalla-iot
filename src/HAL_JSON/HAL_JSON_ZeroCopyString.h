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

#include <cctype>
#include <string>
#include <cstring>
#include <cstdint>

namespace HAL_JSON {

    enum class NumberType { UINT32, INT32, FLOAT, INVALID };

    struct NumberResult {
        NumberType type;
        union {
            uint32_t u32;
            int32_t i32;
            float f32;
        };
    };
    /**
     * Note: 'end' is exclusive, i.e., points just past the last valid character
     */
    struct ZeroCopyString
    {
        const char* start;
        /** Note: 'end' is exclusive, i.e., points just past the last valid character */
        const char* end;
        /** 
         * Returns the lenght of the string,
         * also returns length of zero when either start or end is nullptr
         */
        size_t Length() const;
        inline bool IsEmpty() const {
            return !start || !end || start == end;
        }
        inline bool NotEmpty() const {
            return !IsEmpty();
        }
        /** 
         * Returns a std::string copy of the string, 
         * note this is mostly intended for debug or when the string is needed somewhere else 
         */
        std::string ToString() const;
        /**
         * returns copy of the string as a null terminated char* array using malloc so free must be used
         */
        char* ToCharString() const;
        ZeroCopyString();
        ZeroCopyString(const char* start, const char* end);
        /**
         * Create a ZeroCopyString from a null-terminated C string.
         * 
         * Note: The original string data must remain valid and unchanged
         * for the entire lifetime of the ZeroCopyString instance,
         * as this class does not copy or own the data.
         */
        ZeroCopyString(const char* cstr);
        void Set(const char* cstr);
        void Set(const char* _start, const char* _end);
        void Set(const char* _start, const char* _end, int line, int col); // line and col are just dummy parameters
        bool ContainsPtr(const char* ptr) const;
        /** Returns pointer to first occurrence of ch, or nullptr */
        const char* FindChar(char ch) const;
        /** Returns pointer to first occurrence of ch starting from the end, or nullptr */
        const char* FindCharReverse(char ch) const;
        /** Returns pointer to first occurrence of ch, or nullptr from the start pointer given*/
        const char* FindChar(char ch, const char* start) const;
        bool ContainsChar(char ch) const;
        /** Returns pointer to first occurrence of str, or nullptr from the start pointer given*/
        const char* FindString(const char* str, const char* start = nullptr) const;
        /**
         * @brief Searches for the first occurrence of any string from the given candidate list 
         *        within this ZeroCopyString, optionally starting from the specified position.
         *
         * @param candidates         Null-terminated array of C-strings to search for.
         * @param start              Optional starting position for the search. If nullptr, search begins at the start of the string.
         * @param matchedCandidate   Optional output pointer to receive the candidate string that was matched.
         *                           Set to nullptr if no match is found.
         *
         * @return Pointer to the first match found within the string, or nullptr if none of the candidates match.
         *
         * @note This function preserves the order of the candidate list and returns the first match found.
         *       Useful for locating specific operators or keywords and identifying which one was matched.
         */
        const char* FindAnyString(const char* const* candidates, const char* start, const char** matchedCandidate) const;
        const char* FindFirstMatchingString(const char* const* candidates, const char* _start, const char** matchedCandidate) const;
        uint32_t CountString(const char* str) const;
        uint32_t CountAnyString(const char* const* candidates) const;
        /** Returns a new ZeroCopyString from the given pointers */
        ZeroCopyString substring(const char* sub_start, const char* sub_end) const;
        /** Counts how many times ch appears in the range */
        uint32_t CountChar(char ch) const;
        /** Returns dynamically allocated array of indices (caller must delete[]) */
        const uint32_t* GetIndicies(char ch, uint32_t& outCount) const;
        /** Returns dynamically allocated array of pointers (caller must delete[]) */
        const char** GetPointers(char ch, uint32_t& outCount) const;

        /** 
         * Splits off the leading segment of the string up to the given delimiter. 
         * 
         * Modifies the current ZeroCopyString in-place so that it now starts 
         * just after the delimiter. The extracted head segment is returned 
         * as a new ZeroCopyString.
         * 
         * So the original string need to be copied if one need that later
         * 
         * Example: 
         *   ZeroCopyString z("foo/bar/baz");
         *   ZeroCopyString head = z.SplitOffHead('/');
         *   // head = "foo", z now represents "bar/baz"
         */
        ZeroCopyString SplitOffHead(char delimiter);
        ZeroCopyString SplitOffTail(char delimiter);
        ZeroCopyString SplitOffHead(const char* delimiterPtr);
        ZeroCopyString SplitOffTail(const char* delimiterPtr);

        bool Equals(const ZeroCopyString& other) const;
        bool Equals(const char* cstr) const;
        bool EqualsIC(const ZeroCopyString& other) const;
        bool EqualsIC(const char* cstr) const;
        bool EqualsICAny(const char* const* candidates) const;

        bool ValidNumber() const;
        bool ValidUINT() const;
        bool ConvertTo_uint32(uint32_t& outValue) const;
        bool ConvertTo_int32(int32_t& outValue) const;
        bool ConvertTo_float(float& outValue) const;
        NumberResult ConvertStringToNumber();

        char operator[](size_t idx) const;
    };
    bool operator==(const ZeroCopyString& lhs, const ZeroCopyString& rhs);
    bool operator!=(const ZeroCopyString& lhs, const ZeroCopyString& rhs);
    bool operator==(const char* lhs, const ZeroCopyString& rhs);
    bool operator!=(const char* lhs, const ZeroCopyString& rhs);
    bool operator==(const ZeroCopyString& lhs, const char* rhs);
    bool operator!=(const ZeroCopyString& lhs, const char* rhs);
}