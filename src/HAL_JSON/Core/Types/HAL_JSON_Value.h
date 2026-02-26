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

#include <cstdint>
#include <string>

namespace HAL_JSON {

    class HALValue {
    public:
        enum class Type {TEST, UNSET, UINT, INT, FLOAT/*, STRING*/ };

    private:
        Type type;
        union {
            uint32_t uval;
            float fval;
            int32_t ival;
            //char* str; // future use?? // check chatgpt conversation "Function Parameter Passing"
            //uint8_t* arr; // future use (here the first byte defines how many lenght def bytes that comes after, and then that lenght def bytes def. how many actual data bytes come after that)
        };


    public:
        /** inilializes a HALValue with type set to Unset */
        HALValue();
        HALValue(Type type);
        //HALValue(const HALValue& other);  // future use?? only used for string
        HALValue(uint32_t v);
        HALValue(int32_t v);
        HALValue(float v);
        HALValue(bool v);
        // HALValue(char* str); // future use??

        Type getType() const;
        bool isNumber() const;
        bool isUintOrInt() const;
        bool isNaN() const;

        int32_t asInt() const;
        uint32_t asUInt() const;
        float asFloat() const;

        std::string toString() const;
        void appendToString(std::string& target) const;

        void set(int32_t v);
        void set(uint32_t v);
        void set(float v);

        // Set operators
        HALValue& operator=(int32_t v);
        HALValue& operator=(uint32_t v);
        HALValue& operator=(float v);

        // Conversion Operators
        operator uint8_t() const;
        operator uint32_t() const;
        operator int32_t() const;
        operator float() const;

        HALValue operator+(const HALValue& other) const;
        HALValue operator-(const HALValue& other) const;
        HALValue operator*(const HALValue& other) const;
        HALValue operator/(const HALValue& other) const;
        HALValue operator%(const HALValue& other) const;
        HALValue operator&(const HALValue& other) const;
        HALValue operator|(const HALValue& other) const;
        HALValue operator^(const HALValue& other) const;
        HALValue operator<<(const HALValue& other) const;
        HALValue operator>>(const HALValue& other) const;
    };
    // Comparison Operators
    bool operator==(const HALValue& lhs, const HALValue& rhs);
    bool operator!=(const HALValue& lhs, const HALValue& rhs);
    bool operator<(const HALValue& lhs, const HALValue& rhs);
    bool operator>(const HALValue& lhs, const HALValue& rhs);
    bool operator<=(const HALValue& lhs, const HALValue& rhs);
    bool operator>=(const HALValue& lhs, const HALValue& rhs);

    
}
