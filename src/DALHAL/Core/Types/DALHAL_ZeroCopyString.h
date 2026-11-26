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

#include <cctype>
#include <string>
#include <cstring>
#include <cstdint>
#if defined(ESP8266) || defined(ESP32)
#include <pgmspace.h>
#else
#include <pgmspace.h>
#endif

#include <WString.h> // __FlashStringHelper and F()

// The Arduino cores have __FlashStringHelper as a distinct type. ports/common/WString.h (PC, ESP-IDF)
// defines it to char and sets DALHAL_FLASH_STRING_IS_CHAR, overloads taking it would then collide with the const char* ones
#if !defined(DALHAL_FLASH_STRING_IS_CHAR)
#define DALHAL_ZCS_HAS_FLASH_STRING_TYPE
#endif

namespace DALHAL {

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
     *
     * Flash strings: a ZeroCopyString can be created directly from F("..."), it then points into flash.
     * On all targets except ESP8266 flash is memory mapped and can be read like RAM, so this costs nothing.
     * On ESP8266 flash must be read 32 bits aligned (pgm_read/memcpy_P), so only the following are
     * flash safe there: Length, IsEmpty, NotEmpty, ToString, ToCharString, CopyTo, AppendTo, Equals(const char*).
     * Sinks that read start/end directly must check IsFlash() and copy with CopyTo/AppendTo instead.
     * The parsing functions (Find*, Split*, Trim, Convert* ...) are for RAM text only.
     */
    struct ZeroCopyString
    {
        const char* start;
        /** Note: 'end' is exclusive, i.e., points just past the last valid character */
        const char* end;

#if defined(ESP8266)
        /** true when the string points into flash (irom is mapped at 0x40200000, RAM is below 0x40000000) */
        inline bool IsFlash() const { return reinterpret_cast<uintptr_t>(start) >= 0x40000000u; }
#else
        /** flash is memory mapped on this target, so it is read exactly like RAM */
        static constexpr bool IsFlash() { return false; }
#endif
        /**
         * Copies the string into dst (flash safe) and null terminates it,
         * copies at most dstSize-1 chars, returns the number of chars copied
         */
        size_t CopyTo(char* dst, size_t dstSize) const;
        /** Appends the string to out (flash safe) */
        void AppendTo(std::string& out) const;
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
        /** 
         * Create a ZeroCopyString directly from start and end ptrs, 
         * note. endptr is non inclusive and must allway point to after the last char in the string 
         */
        ZeroCopyString(const char* start, const char* end);
        /** 
         * Create a ZeroCopyString given by start and lenght, 
         * note. endptr is calculated by start+length
         */
        ZeroCopyString(const char* start, size_t length);
        /**
         * Create a ZeroCopyString from a null-terminated C string.
         * 
         * Note: The original string data must remain valid and unchanged
         * for the entire lifetime of the ZeroCopyString instance,
         * as this class does not copy or own the data.
         */
        ZeroCopyString(const char* cstr);
#if defined(DALHAL_ZCS_HAS_FLASH_STRING_TYPE)
        /** Create a ZeroCopyString pointing to a flash string, F("...") (see the flash note at the top) */
        ZeroCopyString(const __FlashStringHelper* fstr);
#endif
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
        bool MoveStartAfter(char delimiter);
        bool MoveEndBefore(char delimiter);
        bool MoveStartAfter(const char* delimiterPtr);
        bool MoveEndBefore(const char* delimiterPtr);
        
        /** just get the substring until the delimiter without changing the original */
        ZeroCopyString GetHead(char delimiter) const;
        /** just get the substring from the delimiterPtr to the end without changing the original */
        ZeroCopyString GetTail(char delimiter) const;
        /** just get the substring until the delimiter without changing the original */
        ZeroCopyString GetHead(const char* delimiterPtr) const;
        /** just get the substring from the delimiterPtr to the end without changing the original */
        ZeroCopyString GetTail(const char* delimiterPtr) const;
        /* trims leading and trailing whitespace from the string*/
        void Trim();

        bool Equals(char c) const;

        bool Equals(const ZeroCopyString& other) const;
        bool Equals(const char* cstr) const;
#if !defined(DALHAL_FLASH_STRING_IS_CHAR)
        bool Equals(const __FlashStringHelper* fstr) const;
#endif
        bool Equals_P(PGM_P pstr) const;

        bool EqualsIC(char c) const;
        bool EqualsIC(const ZeroCopyString& other) const;
        bool EqualsIC(const char* cstr) const;
#if !defined(DALHAL_FLASH_STRING_IS_CHAR)
        bool EqualsIC(const __FlashStringHelper* fstr) const;
#endif
        bool EqualsIC_P(PGM_P pstr) const;

        bool EqualsICAny(const char* const* candidates) const;

        bool StartsWith(char c) const;
        bool StartsWithIC(char c) const;

        /** returns true if any valid number: float, integer or unsigned integer */
        bool ValidNumber() const;
        bool ValidUINT() const;
        /** try to convert to a uint32_t, note allowMinusSign is only used by ConvertTo_int32 to reuse parsing code to convert into signed int */
        bool ConvertTo_uint32(uint32_t& outValue, bool allowMinusSign = false) const;
        bool ConvertTo_int32(int32_t& outValue) const;
        bool ConvertTo_float(float& outValue) const;
        bool ConvertTo_bool(bool& outValue) const;
        NumberResult ConvertStringToNumber() const;

        char operator[](size_t idx) const;
    };

    /** Copies a flash string F("...") into a std::string, use where the text is concatenated or kept */
    inline std::string ToStdString(const __FlashStringHelper* fstr) { return ZeroCopyString(fstr).ToString(); }
    bool operator==(const ZeroCopyString& lhs, const ZeroCopyString& rhs);
    bool operator!=(const ZeroCopyString& lhs, const ZeroCopyString& rhs);
    bool operator==(const char* lhs, const ZeroCopyString& rhs);
    bool operator!=(const char* lhs, const ZeroCopyString& rhs);
    bool operator==(const ZeroCopyString& lhs, const char* rhs);
    bool operator!=(const ZeroCopyString& lhs, const char* rhs);

#if !defined(DALHAL_FLASH_STRING_IS_CHAR)
    bool operator==(const __FlashStringHelper* lhs, const ZeroCopyString& rhs);
    bool operator!=(const __FlashStringHelper* lhs, const ZeroCopyString& rhs);
    bool operator==(const ZeroCopyString& lhs, const __FlashStringHelper* rhs);
    bool operator!=(const ZeroCopyString& lhs, const __FlashStringHelper* rhs);
#endif
}