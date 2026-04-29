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

#include "DALHAL_Value.h"

namespace DALHAL {
    HALValue::HALValue() : type(Type::UNSET) {}
    HALValue::HALValue(Type type) : type(type) {}

    HALValue::HALValue(int32_t v) : type(Type::INT), ival(v) {}
    HALValue::HALValue(uint32_t v) : type(Type::UINT), uval(v) {}
    HALValue::HALValue(float v) : type(Type::FLOAT), fval(v) {}
    HALValue::HALValue(bool v) : type(Type::BOOL), bval(v) {}
    HALValue::HALValue(const char* cStr) : type(Type::CSTRING), cStr(cStr) {}

    // ============================================================================
    // QUERY FUNCTIONS - Inquire about type without conversion
    // ============================================================================

    HALValue::Type HALValue::getType() const {
        return type;
    }

    bool HALValue::isNumber() const {
        return type == Type::UINT || type == Type::FLOAT || type == Type::INT;
    }

    bool HALValue::isUintOrInt() const {
        return type == Type::UINT || type == Type::INT;
    }

    bool HALValue::isNaN() const {
        return !isNumber();
    }

    bool HALValue::isSet() const {
        return type != Type::UNSET;
    }

    // ============================================================================
    // RAW ACCESS - Get the raw value without type conversion
    // Use when you know the type already or want the underlying storage
    // ============================================================================

    int32_t HALValue::asRawInt() const {
        return ival;
    }

    uint32_t HALValue::asRawUInt() const {
        return uval;
    }

    float HALValue::asRawFloat() const {
        return fval;
    }

    bool HALValue::asRawBool() const {
        return bval;
    }

    const char* HALValue::asRawConstChar() const {
        return cStr;
    }

    // ============================================================================
    // CONVERSION FUNCTIONS - Convert to target type intelligently
    // Use when you need a specific type regardless of current storage
    // ============================================================================

    int32_t HALValue::toInt() const {
        switch (type) {
            case Type::INT:
                return ival;
            case Type::UINT:
                return static_cast<int32_t>(uval);
            case Type::FLOAT:
                return static_cast<int32_t>(fval);
            case Type::BOOL:
                return bval ? 1 : 0;
            case Type::CSTRING:
                // Try to parse string as int, return 0 if fail
                return (cStr != nullptr) ? atoi(cStr) : 0;
            default:
                return 0;
        }
    }

    uint32_t HALValue::toUInt() const {
        switch (type) {
            case Type::UINT:
                return uval;
            case Type::INT:
                return static_cast<uint32_t>(ival);
            case Type::FLOAT:
                return static_cast<uint32_t>(fval);
            case Type::BOOL:
                return bval ? 1U : 0U;
            case Type::CSTRING:
                // Try to parse string as uint, return 0 if fail
                return (cStr != nullptr) ? static_cast<uint32_t>(strtoul(cStr, nullptr, 10)) : 0U;
            default:
                return 0U;
        }
    }

    float HALValue::toFloat() const {
        switch (type) {
            case Type::FLOAT:
                return fval;
            case Type::INT:
                return static_cast<float>(ival);
            case Type::UINT:
                return static_cast<float>(uval);
            case Type::BOOL:
                return bval ? 1.0f : 0.0f;
            case Type::CSTRING:
                // Try to parse string as float, return 0 if fail
                return (cStr != nullptr) ? strtof(cStr, nullptr) : 0.0f;
            default:
                return 0.0f;
        }
    }

    bool HALValue::toBool() const {
        switch (type) {
            case Type::BOOL:
                return bval;
            case Type::UINT:
                return uval != 0;
            case Type::INT:
                return ival != 0;
            case Type::FLOAT:
                return fval != 0.0f;
            case Type::CSTRING:
                return cStr != nullptr && cStr[0] != '\0';
            default:
                return false;
        }
    }

    std::string HALValue::toString() const {
        switch (type) {
            case Type::INT:
                return std::to_string(ival);
            case Type::UINT:
                return std::to_string(uval);
            case Type::FLOAT:
                return std::to_string(fval);
            case Type::CSTRING:
                return std::string(cStr != nullptr ? cStr : "");
            case Type::BOOL:
                return bval ? "true" : "false";
            default:
                return "";
        }
    }

    const char* HALValue::toConstChar() const {
        if (type == Type::CSTRING && cStr != nullptr) {
            return cStr;
        }
        return "";
    }

    // ============================================================================
    // STRING BUILDING - Append value to existing string
    // ============================================================================

    void HALValue::appendToString(std::string& target) const {
        char buf[20]; // allocated on the stack
        switch (type) {
            case Type::INT:
                snprintf(buf, sizeof(buf), "%d", ival);
                break;
            case Type::UINT:
                snprintf(buf, sizeof(buf), "%u", uval);
                break;
            case Type::FLOAT:
                snprintf(buf, sizeof(buf), "%f", fval);
                break;
            case Type::BOOL:
                snprintf(buf, sizeof(buf), "%s", bval ? "true" : "false");
                break;
            case Type::CSTRING:
                if (cStr != nullptr) {
                    snprintf(buf, sizeof(buf), "%s", cStr);
                } else {
                    buf[0] = '\0';
                }
                break;
            default:
                buf[0] = '\0';
                break;
        }
        target += buf;
    }

    // ============================================================================
    // ASSIGNMENT OPERATORS
    // ============================================================================

    HALValue& HALValue::operator=(uint32_t v) {
        set(v);
        return *this;
    }

    HALValue& HALValue::operator=(int32_t v) {
        set(v);
        return *this;
    }

    HALValue& HALValue::operator=(float v) {
        set(v);
        return *this;
    }

    // ============================================================================
    // CONVERSION OPERATORS - Implicit conversion (for backwards compatibility)
    // ============================================================================

    HALValue::operator int32_t() const {
        return toInt();
    }

    HALValue::operator uint32_t() const {
        return toUInt();
    }

    HALValue::operator uint8_t() const {
        return static_cast<uint8_t>(toUInt());
    }

    HALValue::operator float() const {
        return toFloat();
    }

    HALValue::operator bool() const {
        return toBool();
    }

    // ============================================================================
    // SET FUNCTIONS
    // ============================================================================

    void HALValue::set(uint32_t v) {
        type = Type::UINT;
        uval = v;
    }

    void HALValue::set(int32_t v) {
        type = Type::INT;
        ival = v;
    }

    void HALValue::set(float v) {
        type = Type::FLOAT;
        fval = v;
    }

    void HALValue::set(bool v) {
        type = Type::BOOL;
        bval = v;
    }

    void HALValue::set(const char* v) {
        type = Type::CSTRING;
        cStr = v;
    }

    // ============================================================================
    // COMPARISON OPERATORS - Use conversion for consistent behavior
    // ============================================================================

    bool operator==(const HALValue& lhs, const HALValue& rhs) {
        // If both are numeric types, compare as floats
        if (lhs.isNumber() && rhs.isNumber()) {
            return lhs.toFloat() == rhs.toFloat();
        }
        // If both are strings, compare string content
        if (lhs.getType() == HALValue::Type::CSTRING && rhs.getType() == HALValue::Type::CSTRING) {
            const char* lstr = lhs.toConstChar();
            const char* rstr = rhs.toConstChar();
            if (lstr == nullptr || rstr == nullptr) {
                return lstr == rstr;
            }
            return std::strcmp(lstr, rstr) == 0;
        }
        // Mixed types: convert to float
        return lhs.toFloat() == rhs.toFloat();
    }

    bool operator!=(const HALValue& lhs, const HALValue& rhs) {
        return !(lhs == rhs);
    }

    bool operator<(const HALValue& lhs, const HALValue& rhs) {
        return lhs.toFloat() < rhs.toFloat();
    }

    bool operator>(const HALValue& lhs, const HALValue& rhs) {
        return lhs.toFloat() > rhs.toFloat();
    }

    bool operator<=(const HALValue& lhs, const HALValue& rhs) {
        return lhs.toFloat() <= rhs.toFloat();
    }

    bool operator>=(const HALValue& lhs, const HALValue& rhs) {
        return lhs.toFloat() >= rhs.toFloat();
    }

    // ============================================================================
    // ARITHMETIC OPERATORS - Type promotion logic
    // ============================================================================

    HALValue HALValue::operator+(const HALValue& other) const {
        // Float promotion: if either is float, result is float
        if (type == Type::FLOAT || other.type == Type::FLOAT) {
            return toFloat() + other.toFloat();
        }
        // Int promotion: if either is signed int, result is signed int
        if (type == Type::INT || other.type == Type::INT) {
            return toInt() + other.toInt();
        }
        // Both unsigned
        return toUInt() + other.toUInt();
    }

    HALValue HALValue::operator-(const HALValue& other) const {
        // Float promotion
        if (type == Type::FLOAT || other.type == Type::FLOAT) {
            return toFloat() - other.toFloat();
        }
        // Int promotion
        if (type == Type::INT || other.type == Type::INT) {
            return toInt() - other.toInt();
        }
        // Both unsigned - avoid underflow
        uint32_t lhs_val = toUInt();
        uint32_t rhs_val = other.toUInt();
        if (rhs_val <= lhs_val) {
            return lhs_val - rhs_val;
        } else {
            return static_cast<int32_t>(lhs_val) - static_cast<int32_t>(rhs_val);
        }
    }

    HALValue HALValue::operator*(const HALValue& other) const {
        // Float promotion
        if (type == Type::FLOAT || other.type == Type::FLOAT) {
            return toFloat() * other.toFloat();
        }
        // Int promotion
        if (type == Type::INT || other.type == Type::INT) {
            return toInt() * other.toInt();
        }
        // Both unsigned
        return toUInt() * other.toUInt();
    }

    HALValue HALValue::operator/(const HALValue& other) const {
        // Float promotion
        if (type == Type::FLOAT || other.type == Type::FLOAT) {
            return toFloat() / other.toFloat();
        }
        // Int promotion
        if (type == Type::INT || other.type == Type::INT) {
            return toInt() / other.toInt();
        }
        // Both unsigned
        return toUInt() / other.toUInt();
    }

    // ============================================================================
    // BITWISE OPERATORS - Operate on integer representation
    // ============================================================================

    HALValue HALValue::operator%(const HALValue& other) const {
        return toUInt() % other.toUInt();
    }

    HALValue HALValue::operator&(const HALValue& other) const {
        return toUInt() & other.toUInt();
    }

    HALValue HALValue::operator|(const HALValue& other) const {
        return toUInt() | other.toUInt();
    }

    HALValue HALValue::operator^(const HALValue& other) const {
        return toUInt() ^ other.toUInt();
    }

    HALValue HALValue::operator<<(const HALValue& other) const {
        return toUInt() << other.toUInt();
    }

    HALValue HALValue::operator>>(const HALValue& other) const {
        return toUInt() >> other.toUInt();
    }

} // namespace DALHAL