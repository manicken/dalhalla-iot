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

#include <DALHAL/API/DALHAL_CommandExecutor.h>
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
    class StreamWriter {
    public:
        enum class DataType {
            Json,
            PlainText
        };
        /**
         * Automatically starts the stream on construction.
         *
         * When the object goes out of scope, the stream is automatically closed.
         *
         * A new block can be started using `restart()` if needed, without
         * destroying the StreamWriter instance.
         * 
         * note. tag MUST point to a valid null-terminated string
         * that remains valid for the entire lifetime of the StreamWriter session.
         */
        StreamWriter(CommandCallback cb, const char* tag, DataType dataType);
        StreamWriter(const StreamWriter&) = delete;
        StreamWriter& operator=(const StreamWriter&) = delete;
        StreamWriter(StreamWriter&&) = delete;

        ~StreamWriter();

        
        void write(const char* data, size_t len);
        inline void write(const ZeroCopyString& zcStr) {
            write(zcStr.start, zcStr.Length());
        }
        inline void write(const char* str) {
            write(str, strlen(str));
        }
        void write(char c);
        
        // tag MUST point to a valid null-terminated string
        // that remains valid for the entire lifetime of the StreamWriter session.
        void restart(const char* tag, DataType dataType);

    private:

        CommandCallback _cb;

        const char* currentTag = nullptr;
        DataType currentDataType = DataType::PlainText;

        char _buf[DALHAL_API_STREAMWRITER_BUFFER_SIZE];
        size_t _pos;

        void sendHeaderFooter(const char* type);
        void start(const char* tag, DataType dataType);
        void flush();
        void end();

        const char* GetCurrentType_cStr();
    };

}