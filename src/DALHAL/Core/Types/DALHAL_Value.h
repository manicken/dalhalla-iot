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
#ifndef DALHAL_VALUE_H
#define DALHAL_VALUE_H

#include <cstring>
#include <string>
#include <cstdlib>

namespace DALHAL {
    /**
     * @class HALValue
     * @brief A type-safe variant that can hold different numeric and string types
     *
     * Design principles:
     * - Query functions (isNumber, isSet, etc) examine type without conversion
     * - asRaw*() functions provide direct access to underlying storage
     * - to*() functions intelligently convert to target type
     * - All operators use to*() for consistent type promotion
     */
    class HALValue {
    public:
        enum class Type {
            TEST, // test is a special type that is used check device functionality
            UNSET,
            INT,
            UINT,
            FLOAT,
            BOOL,
            CSTRING
        };

        // ====================================================================
        // CONSTRUCTORS
        // ====================================================================
        HALValue();
        explicit HALValue(Type type);
        HALValue(int32_t v);
        HALValue(uint32_t v);
        HALValue(float v);
        HALValue(bool v);
        HALValue(const char* cStr);

        // ====================================================================
        // QUERY FUNCTIONS - Ask about type without conversion
        // ====================================================================
        /**
         * @brief Get the current type
         */
        Type getType() const;

        /**
         * @brief Check if value is a numeric type (INT, UINT, or FLOAT)
         */
        bool isNumber() const;

        /**
         * @brief Check if value is INT or UINT
         */
        bool isUintOrInt() const;

        /**
         * @brief Check if value is NOT a number (inverse of isNumber)
         */
        bool isNaN() const;

        /**
         * @brief Check if value has been set (not UNSET)
         */
        bool isSet() const;

        // ====================================================================
        // RAW ACCESS - Direct access to underlying storage
        // Use when you know the type or want the raw value as-is
        // ====================================================================
        /**
         * @brief Get raw int32_t value without conversion
         */
        int32_t asRawInt() const;

        /**
         * @brief Get raw uint32_t value without conversion
         */
        uint32_t asRawUInt() const;

        /**
         * @brief Get raw float value without conversion
         */
        float asRawFloat() const;

        /**
         * @brief Get raw bool value without conversion
         */
        bool asRawBool() const;

        /**
         * @brief Get raw C-string pointer without conversion
         */
        const char* asRawConstChar() const;

        // ====================================================================
        // CONVERSION FUNCTIONS - Intelligent type conversion
        // Use when you need a specific type regardless of current storage
        // ====================================================================
        /**
         * @brief Convert value to int32_t
         * - INT → direct return
         * - UINT/FLOAT/BOOL → cast
         * - CSTRING → parse with atoi() (0 if parse fails)
         */
        int32_t toInt() const;

        /**
         * @brief Convert value to uint32_t
         * - UINT → direct return
         * - INT/FLOAT/BOOL → cast
         * - CSTRING → parse with strtoul() (0 if parse fails)
         */
        uint32_t toUInt() const;

        /**
         * @brief Convert value to float
         * - FLOAT → direct return
         * - INT/UINT/BOOL → cast
         * - CSTRING → parse with strtof() (0.0f if parse fails)
         */
        float toFloat() const;

        /**
         * @brief Convert value to bool
         * - BOOL → direct return
         * - INT/UINT → check != 0
         * - FLOAT → check != 0.0f
         * - CSTRING → check non-null and non-empty
         */
        bool toBool() const;

        /**
         * @brief Convert value to std::string
         * - INT/UINT → std::to_string
         * - FLOAT → std::to_string
         * - CSTRING → convert to string
         * - BOOL → "true" or "false"
         */
        std::string toString() const;

        /**
         * @brief Get value as const char*
         * - CSTRING → return pointer
         * - other → return empty string ""
         */
        const char* toConstChar() const;

        // ====================================================================
        // STRING BUILDING
        // ====================================================================
        /**
         * @brief Append this value to a string
         */
        void appendToString(std::string& target) const;

        // ====================================================================
        // SETTER FUNCTIONS
        // ====================================================================
        void set(uint32_t v);
        void set(int32_t v);
        void set(float v);
        void set(bool v);
        void set(const char* v);

        // ====================================================================
        // ASSIGNMENT OPERATORS
        // ====================================================================
        HALValue& operator=(uint32_t v);
        HALValue& operator=(int32_t v);
        HALValue& operator=(float v);

        // ====================================================================
        // CONVERSION OPERATORS (implicit - use to*() for explicit)
        // ====================================================================
        explicit operator int32_t() const;
        explicit operator uint32_t() const;
        explicit operator uint8_t() const;
        explicit operator float() const;
        explicit operator bool() const;

        // ====================================================================
        // ARITHMETIC OPERATORS
        // Type promotion rules:
        // - Float: if either is FLOAT, result is float
        // - Int:   if either is INT, result is int
        // - UInt:  both are unsigned → result is unsigned
        // ====================================================================
        HALValue operator+(const HALValue& other) const;
        HALValue operator-(const HALValue& other) const;
        HALValue operator*(const HALValue& other) const;
        HALValue operator/(const HALValue& other) const;

        // ====================================================================
        // BITWISE OPERATORS - Convert to uint then operate
        // ====================================================================
        HALValue operator%(const HALValue& other) const;
        HALValue operator&(const HALValue& other) const;
        HALValue operator|(const HALValue& other) const;
        HALValue operator^(const HALValue& other) const;
        HALValue operator<<(const HALValue& other) const;
        HALValue operator>>(const HALValue& other) const;

    private:
        Type type;

        // Union-style storage (only one is valid at a time based on type)
        union {
            int32_t ival;
            uint32_t uval;
            float fval;
            bool bval;
            const char* cStr;
        };
    };

    // ========================================================================
    // COMPARISON OPERATORS
    // ========================================================================
    /**
     * @brief Equality comparison
     * - Both numeric: compare as float
     * - Both string: compare string content
     * - Mixed: convert to float
     */
    bool operator==(const HALValue& lhs, const HALValue& rhs);

    /**
     * @brief Inequality comparison
     */
    bool operator!=(const HALValue& lhs, const HALValue& rhs);

    /**
     * @brief Less than comparison (uses float conversion)
     */
    bool operator<(const HALValue& lhs, const HALValue& rhs);

    /**
     * @brief Greater than comparison (uses float conversion)
     */
    bool operator>(const HALValue& lhs, const HALValue& rhs);

    /**
     * @brief Less than or equal comparison (uses float conversion)
     */
    bool operator<=(const HALValue& lhs, const HALValue& rhs);

    /**
     * @brief Greater than or equal comparison (uses float conversion)
     */
    bool operator>=(const HALValue& lhs, const HALValue& rhs);

} // namespace DALHAL

#endif // DALHAL_VALUE_H