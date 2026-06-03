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
    void StringBuilderStreamer::write(char c) {
        if (_pos >= sizeof(_buf)) flush();
        _buf[_pos++] = c;
    }

    void StringBuilderStreamer::write(bool v) {
        if (v) {
            write(F("true"));
        } else {
            write(F("false"));
        }
    }

    void StringBuilderStreamer::write(const __FlashStringHelper* fstr) {
        return write_P(reinterpret_cast<PGM_P>(fstr));
    }
    void StringBuilderStreamer::write(const __FlashStringHelper* fstr, size_t len) {
        return write_P(reinterpret_cast<PGM_P>(fstr), len);
    }
    void StringBuilderStreamer::write_P(PGM_P pstr) {
        if (!pstr) {
            return;
        }
        size_t i = 0;
        char b=1;
        while (true) {
            b=pgm_read_byte(pstr + i);
            if (b == '\0') break;
            write(b);
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
            write(b);
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

    void StringBuilderStreamer::write_asBin(uint8_t v) {
        for (int bit = 7; bit >= 0; --bit) {
            write((v & (1U << bit)) ? '1' : '0');
        }
    }
    void StringBuilderStreamer::write_asBin(uint16_t v) {
        for (int bit = 15; bit >= 0; --bit) {
            write((v & (1U << bit)) ? '1' : '0');
        }
    }
    void StringBuilderStreamer::write_asBin(uint32_t v) {
        for (int bit = 31; bit >= 0; --bit) {
            write((v & (1U << bit)) ? '1' : '0');
        }
    }
    char NibbleToHexChar(uint8_t value)
    {
        value &= 0x0F;
        if (value>9) return (value - 10) + 'A';
        else return value + '0';
    }
    void StringBuilderStreamer::write_asHex(uint8_t v) {
        write(NibbleToHexChar(v >> 4));
        write(NibbleToHexChar(v));
    }
    void StringBuilderStreamer::write_asHex(uint16_t v) {
        write_asHex((uint8_t)(v >> 8));
        write_asHex((uint8_t)(v & 0xFF));
    }
    void StringBuilderStreamer::write_asHex(uint32_t v) {
        write_asHex((uint16_t)(v >> 16));
        write_asHex((uint16_t)(v & 0xFFFF));
    }

    void StringBuilderStreamer::write_json(float v) {
        if (isnan(v) || isfinite(v)) {
            write(F("null"), 4); // otherwise it will print nan which is a invalid json type
            return;
        }
        char buf[32];
        int n = snprintf(buf, sizeof(buf), "%g", (double)v);
        if (n > 0) write(buf, (size_t)n);
    }

    void StringBuilderStreamer::write_jsonQuoted(const __FlashStringHelper* fstr, size_t len) {
        write('"');
        write(fstr, len);
        write('"');
    }
    void StringBuilderStreamer::write_jsonQuoted(const __FlashStringHelper* fstr) {
        write('"');
        write(fstr);
        write('"');
    }
    void StringBuilderStreamer::write_jsonQuoted(const char* cstr, size_t len) {
        write('"');
        write(cstr, len);
        write('"');
    }
    void StringBuilderStreamer::write_jsonQuoted(const char* cstr) {
        write_jsonQuoted(cstr, strlen(cstr));
    }

    void StringBuilderStreamer::write_jsonKey(const __FlashStringHelper* fstr, size_t len) {
        write_jsonQuoted(fstr, len);
        write(':');
    }
    void StringBuilderStreamer::write_jsonKey(const __FlashStringHelper* fstr) {
        write_jsonQuoted(fstr);
        write(':');
    }

    void StringBuilderStreamer::write_jsonKey(const char* cstr, size_t len) {
        write_jsonQuoted(cstr, len);
        write(':');
    }
    void StringBuilderStreamer::write_jsonKey(const char* cstr) {
        write_jsonKey(cstr, strlen(cstr));
    }

    void StringBuilderStreamer::write_jsonString(const __FlashStringHelper* key, const char* cstr) {
        write_jsonKey(key);
        write_jsonQuoted(cstr);
    }
    void StringBuilderStreamer::write_jsonString(const __FlashStringHelper* key, const ZeroCopyString& zcStr) {
        write_jsonKey(key);
        write_jsonQuoted(zcStr.start, zcStr.Length());
    }
    void StringBuilderStreamer::write_jsonString(const __FlashStringHelper* key, const __FlashStringHelper* fstr) {
        write_jsonKey(key);
        write_jsonQuoted(fstr);
    }

    void StringBuilderStreamer::write_jsonBool(const __FlashStringHelper* key, bool v) {
        write_jsonKey(key);
        write(v);
    }
    void StringBuilderStreamer::write_jsonNumber(const __FlashStringHelper* key, uint32_t v) {
        write_jsonKey(key);
        write(v);
    }
    void StringBuilderStreamer::write_jsonNumber(const __FlashStringHelper* key, int32_t v) {
        write_jsonKey(key);
        write(v);
    }
    void StringBuilderStreamer::write_jsonNumber(const __FlashStringHelper* key, float v) {
        write_jsonKey(key);
        write_json(v);
    }

}