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

#include "ConvertHelper.h"

#include "HAL_JSON/Support/HAL_JSON_Logger.h"

namespace Convert
{
    std::string floatToString(float value) {
        char buffer[32]; // Adjust size based on needs
        snprintf(buffer, sizeof(buffer), "%.6f", value); // Format as float with 6 decimals

        std::string result(buffer);

        // Remove trailing zeros
        result.erase(result.find_last_not_of('0') + 1, std::string::npos);
        if (result.back() == '.') {
            result.pop_back(); // Remove decimal point if no fractional part
        }

        return result;
    }

    bool isInteger(const char* str)
    {
        if (str == nullptr || *str == '\0') return false; // Null or empty string is not valid

        // Handle leading sign
        if (*str == '+' || *str == '-') str++;

        // Check the rest of the string
        while (*str != '\0')
        {
            if (!isdigit(*str)) return false;
            str++;
        }

        return true; // All characters are digits
    }

    char ConvertOneNibble(uint8_t value)
    {
        if (value>9) return (value - 10) + 'A';
        else return value + '0';
    }

    std::string toHex(uint8_t value)
    {
        char hexStr[3];
        hexStr[0] = ConvertOneNibble((value >> 4) & 0x0F);
        hexStr[1] = ConvertOneNibble(value & 0x0F);
        hexStr[2] = 0x00;
        return std::string(hexStr);
    }

    std::string toHex(uint16_t value) {
        char hexStr[5];
        uint32_t divider = 0x1000;
        for (int i=0;i<4;i++) {
            uint32_t nibble = (value/divider)&0xF;
            hexStr[i] = (nibble >= 10) ? (nibble + ('A'-10)) : (nibble + '0');
            value %=divider;
            divider>>=4;
        }
        hexStr[4] = 0x00;
        return std::string(hexStr);
    }

    std::string toHex(uint32_t value) {
        char hexStr[9];
        uint32_t divider = 0x10000000;
        for (int i=0;i<8;i++) {
            uint32_t nibble = (value/divider)&0xF;
            hexStr[i] = (nibble >= 10) ? (nibble + ('A'-10)) : (nibble + '0');
            value %=divider;
            divider>>=4;
        }
        hexStr[8] = 0x00;
        return std::string(hexStr);
    }

    std::string toHex(uint64_t value) {
        char hexStr[17];
        uint64_t divider = 0x1000000000000000;
        for (int i=0;i<16;i++) {
            uint32_t nibble = (value/divider)&0xF;
            hexStr[i] = (nibble >= 10) ? (nibble + ('A'-10)) : (nibble + '0');
            value %=divider;
            divider>>=4;
        }
        hexStr[16] = 0x00;
        return std::string(hexStr);
    }

    std::string toBin(uint8_t value) {
        char bitStr[9];
        uint32_t divider = 0x80;
        for (int i=0;i<8;i++) {
            bitStr[i] = ((value / divider) & 0x01) + '0';
            value %= divider;
            divider >>=1;
        }
        bitStr[8] = 0;
        return std::string(bitStr);
    }

    std::string ByteArrayToString(uint8_t* byteArray, size_t arraySize)
    {
        std::string str = "";
        for (size_t i=0;i<arraySize;i++) {
            str.append(toHex(byteArray[i]));
            if (i<(arraySize-1)) str.append(":");
        }
        return str;
    }

    bool HexToBytes(const char* hexString, uint8_t* byteArray, size_t arraySize)
    {
        if (!hexString) { // if byteArray is nullptr then this function is only used to verify a hexString
            GlobalLogger.Error(F("HexToBytes - Invalid input 1"));
            //DEBUG_UART.println(F("HexToBytes - Invalid input 1"));
            return false; // Invalid input 1
        }
        size_t hexStrLen = strlen(hexString);
        int incr = 0;
        if (hexStrLen == arraySize*2) // no deliminator between hex-byte numbers
            incr = 2;
        else if (hexStrLen == (arraySize*3)-1)// any desired deliminator between hex-byte numbers
            incr = 3;
        else {
            GlobalLogger.Error(F("HexToBytes - hexStrLen mismatch:"),std::to_string(hexStrLen).c_str());
           // DEBUG_UART.print(F("HexToBytes - hexStrLen mismatch:"));
            //DEBUG_UART.println(String(hexStrLen));
            return false; // Invalid input 2
        }
        size_t byteIndex = 0;
        for (size_t i = 0; i < hexStrLen; i+=incr) {
            if (byteIndex >= arraySize) {
                GlobalLogger.Error(F("HexToBytes - Exceeded array size"));
                //DEBUG_UART.println(F("HexToBytes - Exceeded array size"));
                return false; // Exceeded array size
            }
            char highNibble = hexString[i];
            char lowNibble = hexString[i + 1];

            if (!std::isxdigit(highNibble) || !std::isxdigit(lowNibble)) {
                GlobalLogger.Error(F("HexToBytes - Non-hex character found"));
                //DEBUG_UART.println(F("HexToBytes - Non-hex character found"));
                return false; // Non-hex character found
            }
            if (byteArray != nullptr) // if byteArray is nullptr then this is only a convert pre-validate
                byteArray[byteIndex++] = (uint8_t)((std::isdigit(highNibble) ? highNibble - '0' : std::toupper(highNibble) - 'A' + 10) << 4 |
                                    (std::isdigit(lowNibble) ? lowNibble - '0' : std::toupper(lowNibble) - 'A' + 10));
        }

        return true; // Conversion successful
    }

    uint64_t reverseMACaddress(uint64_t addr)
    {
        uint8_t* macAddrPtr = reinterpret_cast<uint8_t*>(&addr);
        uint64_t macAddrBigEndian = (macAddrPtr[0] * 0x10000000000) + 
                                    (macAddrPtr[1] * 0x00100000000) + 
                                    (macAddrPtr[2] * 0x00001000000) + 
                                    (macAddrPtr[3] * 0x00000010000) + 
                                    (macAddrPtr[4] * 0x00000000100) + 
                                    (macAddrPtr[5]);
        return macAddrBigEndian;
    }

    String toHex(const char *data, size_t len) {
        String hex = "";
        for (size_t i = 0; i < len; i++) {
            if (data[i] < 16) hex += "0";  // Pad single digits
            hex += String(data[i], HEX);
            if (i<len-1) hex += ",";
        }
        return hex;
    }

    String convertISO88591toUTF8(const String &input) {
        String output = "";
        for (unsigned int i = 0; i < input.length(); i++) {
            unsigned char c = input[i];
            if (c == 1) output += "\u2581";
            else if (c == 2) output += "\u2582";
            else if (c == 3) output += "\u2583";
            else if (c == 4) output += "\u2584";
            else if (c == 5) output += "\u2585";
            else if (c == 6) output += "\u2586";
            else if (c == 7) output += "\u2587";
            else if (c == 8) output += "\u2588";
            else if (c < 32) { output += "{"; output += Convert::toHex(c).c_str(); output+="}"; }
            else if (c < 128) {
            output += (char)c;
            } else {
            output += (char)(0xC0 | (c >> 6));
            output += (char)(0x80 | (c & 0x3F));
            }
        }
        return output;
    }

    bool IsValidHexString(const char* str) {
        if (str == nullptr || strlen(str) == 0) return false; // failsafe
        for (;*str;str++) {
            if (!std::isxdigit(static_cast<unsigned char>(*str))) {
                return false;
            }
        }
        return true;
    }

    

}