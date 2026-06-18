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

#include "DALHAL_StringBuilderStreamer.h"

#include <DALHAL/Support/DALHAL_Logger.h>

#include <math.h>

namespace DALHAL {

    StringBuilderStreamer::StringBuilderStreamer(StreamCallback cb)
        : _cb(cb), _pos(0) { }

    StringBuilderStreamer::~StringBuilderStreamer() {
        flush();
    }

    /* private */
    void StringBuilderStreamer::flush() {
        if (_pos == 0) return;
        _cb(_buf, _pos);
        _pos = 0;
    }

//    ██     ██ ██████  ██ ████████ ███████     ███████ ██    ██ ███    ██  ██████ ████████ ██  ██████  ███    ██ ███████ 
//    ██     ██ ██   ██ ██    ██    ██          ██      ██    ██ ████   ██ ██         ██    ██ ██    ██ ████   ██ ██      
//    ██  █  ██ ██████  ██    ██    █████       █████   ██    ██ ██ ██  ██ ██         ██    ██ ██    ██ ██ ██  ██ ███████ 
//    ██ ███ ██ ██   ██ ██    ██    ██          ██      ██    ██ ██  ██ ██ ██         ██    ██ ██    ██ ██  ██ ██      ██ 
//     ███ ███  ██   ██ ██    ██    ███████     ██       ██████  ██   ████  ██████    ██    ██  ██████  ██   ████ ███████ 

    bool StringBuilderStreamer::writef(const char* fmt, ...)
    {
        if (!fmt) return false;

        va_list args;
        va_start(args, fmt);

        while (true)
        {
            size_t remaining = sizeof(_buf) - _pos;

            if (remaining == 0)
            {
                flush();
                continue;
            }

            va_list args_copy;
            va_copy(args_copy, args);

            int n = vsnprintf(_buf + _pos, remaining, fmt, args_copy);

            va_end(args_copy);

            if (n < 0)
            {
                va_end(args);
                GlobalLogger.Error(F("SBS format error"));
                return false;
            }

            // real "impossible to ever fit"
            if ((size_t)n > sizeof(_buf))
            {
                va_end(args);
                GlobalLogger.Error(F("SBS formatted message too large"));
                return false;
            }

            // doesn't fit in remaining → flush and retry
            if ((size_t)n >= remaining)
            {
                flush();
                continue;
            }

            // fits
            _pos += (size_t)n;
            va_end(args);
            return true;
        }
    }

    /* public */
    void StringBuilderStreamer::write(const char* data, size_t len) {
        while (len > 0) {

            size_t space = sizeof(_buf) - _pos;
            if (space == 0) flush();

            size_t toCopy = (len < space) ? len : space;

            memcpy(_buf + _pos, data, toCopy);

            _pos += toCopy;
            data += toCopy;
            len -= toCopy;
        }
    }

    /* public */
    void StringBuilderStreamer::write_char(char c) {
        if (c == '\0') return;

        if (_pos >= sizeof(_buf)) flush();
        _buf[_pos++] = c;
    }
    void StringBuilderStreamer::write_2chars(char a, char b)
    {
        if (_pos + 2 >= sizeof(_buf)) flush();
        _buf[_pos++] = a;
        _buf[_pos++] = b;
    }
    char NibbleToHexCharLowerCase(uint8_t value)
    {
        value &= 0x0F;
        if (value>9) return (value - 10) + 'a';
        else return value + '0';
    }
    void StringBuilderStreamer::write_escaped(char c) {
        if (c == '\0') return;

        switch (c)
        {
            case '"':  write_2chars('\\', '"'); break;
            case '\\': write_2chars('\\', '\\'); break;
            case '\n': write_2chars('\\', 'n');  break;
            case '\r': write_2chars('\\', 'r');  break;
            case '\t': write_2chars('\\', 't');  break;
            case '\b': write_2chars('\\', 'b');  break;
            case '\f': write_2chars('\\', 'f');  break;

            default:
                // control chars
                if ((unsigned char)c < 0x20)
                {
                    // safest fallback
                    // write chars by chars to avoid using slow PROGMEM 
                    // (specially on ESP8266 where F() strings are needed to save ram)
                    write_2chars('\\', 'u'); write_2chars('0', '0');
                    uint8_t v = (uint8_t)c;
                    write_2chars(NibbleToHexCharLowerCase(v >> 4), NibbleToHexCharLowerCase(v));
                }
                else
                {
                    write_char((char)c);
                }
                break;
        }
    }

    void StringBuilderStreamer::write(bool v) {
        if (v) {
            write(F("true"));
        } else {
            write(F("false"));
        }
    }
#if !(defined(_WIN32) || defined(__linux__) || defined(__APPLE__))
    void StringBuilderStreamer::write(const __FlashStringHelper* fstr) {
        return write_P(reinterpret_cast<PGM_P>(fstr));
    }
    void StringBuilderStreamer::write(const __FlashStringHelper* fstr, size_t len) {
        return write_P(reinterpret_cast<PGM_P>(fstr), len);
    }
#endif
    void StringBuilderStreamer::write_P(PGM_P pstr) {
        if (!pstr) {
            return;
        }
        size_t i = 0;
        char b=1;
        while (true) {
            b=pgm_read_byte(pstr + i);
            if (b == '\0') break;
            write_char(b);
            i++;
        }
    }
    void StringBuilderStreamer::write_escapedChars_P(PGM_P pstr) {
        if (!pstr) {
            return;
        }
        size_t i = 0;
        char b=1;
        while (true) {
            b=pgm_read_byte(pstr + i);
            if (b == '\0') break;
            write_escaped(b);
            i++;
        }
    }

    void StringBuilderStreamer::write_P(PGM_P pstr, size_t len) {
        if (!pstr) {
            return;
        }

        if (len == 0) {
            return;
        }

        for (size_t i = 0; i < len; i++) {
            char b = pgm_read_byte(pstr + i);
            write_char(b);
        }
    }

    void StringBuilderStreamer::write_escapedChars_P(PGM_P pstr, size_t len) {
        if (!pstr) {
            return;
        }

        if (len == 0) {
            return;
        }

        for (size_t i = 0; i < len; i++) {
            char b = pgm_read_byte(pstr + i);
            write_escaped(b);
        }
    }

    void StringBuilderStreamer::write(uint32_t v) {
        char buf[11]; // max 4294967295 + '\0'
        int n = snprintf(buf, sizeof(buf), "%lu", (unsigned long)v);
        if (n > 0) write(buf, (size_t)n);
    }
    void StringBuilderStreamer::write(int32_t v) {
        char buf[12]; // max -2147483648 + '\0'
        int n = snprintf(buf, sizeof(buf), "%ld", (long)v);
        if (n > 0) write(buf, (size_t)n);
    }
    void StringBuilderStreamer::write(float v) {
        char buf[32];
        int n = snprintf(buf, sizeof(buf), "%g", (double)v);
        if (n > 0) write(buf, (size_t)n);
    }

    void StringBuilderStreamer::write(uint32_t v, const char* fmt) {
        char buf[11]; // max 4294967295 + '\0'
        int n = snprintf(buf, sizeof(buf), fmt, (unsigned long)v);
        if (n > 0) write(buf, (size_t)n);
    }
    void StringBuilderStreamer::write(int32_t v, const char* fmt) {
        char buf[11]; // max 4294967295 + '\0'
        int n = snprintf(buf, sizeof(buf), fmt, (unsigned long)v);
        if (n > 0) write(buf, (size_t)n);
    }

    void StringBuilderStreamer::write_asBin(uint8_t v) {
        for (int bit = 7; bit >= 0; --bit) {
            write_char((v & (1U << bit)) ? '1' : '0');
        }
    }
    void StringBuilderStreamer::write_asBin(uint16_t v) {
        for (int bit = 15; bit >= 0; --bit) {
            write_char((v & (1U << bit)) ? '1' : '0');
        }
    }
    void StringBuilderStreamer::write_asBin(uint32_t v) {
        for (int bit = 31; bit >= 0; --bit) {
            write_char((v & (1U << bit)) ? '1' : '0');
        }
    }
    char NibbleToHexChar(uint8_t value)
    {
        value &= 0x0F;
        if (value>9) return (value - 10) + 'A';
        else return value + '0';
    }
    
    void StringBuilderStreamer::write_asHex(uint8_t v) {
        write_char(NibbleToHexChar(v >> 4));
        write_char(NibbleToHexChar(v));
    }
    void StringBuilderStreamer::write_asHex(uint16_t v) {
        write_asHex((uint8_t)(v >> 8));
        write_asHex((uint8_t)(v & 0xFF));
    }
    void StringBuilderStreamer::write_asHex(uint32_t v) {
        write_asHex((uint16_t)(v >> 16));
        write_asHex((uint16_t)(v & 0xFFFF));
    }

    void StringBuilderStreamer::write_asHex(uint8_t* buff, size_t len, char separator /* = '\0'*/) {
        for (size_t i=0;i<len;i++) {
            if (separator != '\0' && i > 0) {
                write_char(separator);
            }
            write_asHex(buff[i]);
        }
    }

    void StringBuilderStreamer::write_json(float v) {
        if (isnan(v) || !isfinite(v)) {
            write(F("null")); // otherwise it will print nan which is a invalid json type
            return;
        }
        char buf[32];
        int n = snprintf(buf, sizeof(buf), "%g", (double)v);
        if (n > 0) write(buf, (size_t)n);
    }

    void StringBuilderStreamer::write_json_value_separator() {
        write_char(',');
    }
    void StringBuilderStreamer::write_json_member_separator() {
        write_char(':');
    }
    void StringBuilderStreamer::write_json_array_begin() {
        write_char('[');
    }
    void StringBuilderStreamer::write_json_array_end() {
        write_char(']');
    }
    void StringBuilderStreamer::write_json_object_begin() {
        write_char('{');
    }
    void StringBuilderStreamer::write_json_object_end() {
        write_char('}');
    }
#if !(defined(_WIN32) || defined(__linux__) || defined(__APPLE__))
    void StringBuilderStreamer::write_jsonQuoted(const __FlashStringHelper* fstr) {
        if (fstr == nullptr) {
            write(F("null"));
            return;
        }
        write_doublequote();
        write_escapedChars_P(reinterpret_cast<PGM_P>(fstr));
        write_doublequote();
    }
#endif
    void StringBuilderStreamer::write_jsonQuoted(const char* cstr, size_t len) {
        if (!cstr || len == 0)
        {
            write(F("null"));
            return;
        }
        write_doublequote();
 
            size_t i=0;
            for (const char* p = cstr; (*p != '\0') && (i < len); ++p, ++i)
            {
                write_escaped(*p);
            }

        write_doublequote();
    }
    void StringBuilderStreamer::write_jsonQuoted(const char* cstr) {
        if (!cstr)
        {
            write(F("null"));
            return;
        }

        write_doublequote();
        //Serial1.printf("cstr = %p\n", cstr);

            for (const char* p = cstr; *p != '\0'; ++p)
            {
                write_escaped(*p);
            }
        

        write_doublequote();
    }
#if !(defined(_WIN32) || defined(__linux__) || defined(__APPLE__))
    void StringBuilderStreamer::write_jsonMemberStart(const __FlashStringHelper* fstr) {
        write_jsonQuoted(fstr);
        write_json_member_separator();
    }
#endif
    void StringBuilderStreamer::write_jsonMemberStart(const char* cstr, size_t len) {
        write_jsonQuoted(cstr, len);
        write_json_member_separator();
    }
    void StringBuilderStreamer::write_jsonMemberStart(const char* cstr) {
        write_jsonMemberStart(cstr, strlen(cstr));
    }

    void StringBuilderStreamer::write_jsonString(const __FlashStringHelper* key, const char* cstr) {
        write_jsonMemberStart(key);
        write_jsonQuoted(cstr);
    }
    void StringBuilderStreamer::write_jsonString(const __FlashStringHelper* key, const ZeroCopyString& zcStr) {
        write_jsonMemberStart(key);
        write_jsonQuoted(zcStr.start, zcStr.Length());
    }
#if !(defined(_WIN32) || defined(__linux__) || defined(__APPLE__))
    void StringBuilderStreamer::write_jsonString(const __FlashStringHelper* key, const __FlashStringHelper* fstr) {
        write_jsonMemberStart(key);
        write_jsonQuoted(fstr);
    }
#endif
    void StringBuilderStreamer::write_jsonBool(const __FlashStringHelper* key, bool v) {
        write_jsonMemberStart(key);
        write(v);
    }
    void StringBuilderStreamer::write_jsonNumber(const __FlashStringHelper* key, uint32_t v) {
        write_jsonMemberStart(key);
        write(v);
    }
    void StringBuilderStreamer::write_jsonNumber(const __FlashStringHelper* key, int32_t v) {
        write_jsonMemberStart(key);
        write(v);
    }
    void StringBuilderStreamer::write_jsonNumber(const __FlashStringHelper* key, float v) {
        write_jsonMemberStart(key);
        write_json(v);
    }

}