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

#include <functional>
#include <DALHAL/Core/Types/DALHAL_ZeroCopyString.h>

#if defined(ESP8266)
#define DALHAL_API_STREAMWRITER_BUFFER_SIZE 512
#elif defined(ESP32)
#define DALHAL_API_STREAMWRITER_BUFFER_SIZE 1024
#elif defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
#define DALHAL_API_STREAMWRITER_BUFFER_SIZE 1024*1024
#else
#define DALHAL_API_STREAMWRITER_BUFFER_SIZE 256
#endif

#define DALHAL_API_STREAMWRITER_HEADER_FOOTER_BUFFER_SIZE 128

#if DALHAL_API_STREAMWRITER_BUFFER_SIZE < DALHAL_API_STREAMWRITER_HEADER_FOOTER_BUFFER_SIZE
#define DALHAL_API_STREAMWRITER_USE_SEPARATE_HEADER_FOOTER_BUFFER
#endif

namespace DALHAL {
    class StringBuilderStreamer {
    public:

        using StreamCallback = std::function<bool(const char* buf, size_t len)>;

        StringBuilderStreamer (StreamCallback cb);
        StringBuilderStreamer (const StringBuilderStreamer &) = delete;
        StringBuilderStreamer & operator=(const StringBuilderStreamer &) = delete;
        StringBuilderStreamer (StringBuilderStreamer &&) = delete;

        ~StringBuilderStreamer();

        void flush();

        void write(const char* data, size_t len);
        
        inline void write(const ZeroCopyString& zcStr) {
            write(zcStr.start, zcStr.Length());
        }
        inline void write(const char* str) {
            write(str, strlen(str));
        }
        void write(const __FlashStringHelper* fstr);
        void write(const __FlashStringHelper* fstr, size_t len);
        void write_P(PGM_P pstr, size_t len);
        void write_P(PGM_P pstr);

        void write(char c);
        void write(bool v);
        void write(uint32_t v);
        void write(int32_t v);
        void write(float v);

        void write_asBin(uint8_t v);
        void write_asBin(uint16_t v);
        void write_asBin(uint32_t v);
        void write_asHex(uint8_t v);
        void write_asHex(uint16_t v);
        void write_asHex(uint32_t v);
        
        void write_json(float v);
        void write_jsonQuoted(const __FlashStringHelper* fstr);
        void write_jsonQuoted(const __FlashStringHelper* fstr, size_t len);
        void write_jsonQuoted(const char* cstr, size_t len);
        void write_jsonQuoted(const char* cstr);
        void write_jsonKey(const __FlashStringHelper* fstr);
        void write_jsonKey(const __FlashStringHelper* fstr, size_t len);
        void write_jsonKey(const char* cstr, size_t len);
        void write_jsonKey(const char* cstr);

        void write_jsonString(const __FlashStringHelper* key, const char* cstr);
        void write_jsonString(const __FlashStringHelper* key, const ZeroCopyString& zcStr);
        void write_jsonString(const __FlashStringHelper* key, const __FlashStringHelper* fstr);
        void write_jsonBool(const __FlashStringHelper* key, bool v);
        void write_jsonNumber(const __FlashStringHelper* key, uint32_t v);
        void write_jsonNumber(const __FlashStringHelper* key, int32_t v);
        void write_jsonNumber(const __FlashStringHelper* key, float v);

    private:

        StreamCallback _cb;

        char _buf[DALHAL_API_STREAMWRITER_BUFFER_SIZE];
        size_t _pos;

        

    };

}