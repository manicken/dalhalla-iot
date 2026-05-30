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

#include "DALHAL_API_StreamWriter.h"

namespace DALHAL {

    StreamWriter::StreamWriter(CommandCallback cb, const char* tag, DataType dataType)
        : _cb(cb), _pos(0)
    {
        start(tag, dataType);
    }

    StreamWriter::~StreamWriter() {
        end();
    }

    /* private */
    const char* StreamWriter::GetCurrentType_cStr() {
        if (currentDataType == DataType::Json) {
            return "json";
        } else if (currentDataType == DataType::PlainText) {
            return "text";
        } else {
            return "unknown";
        }
    }

    /* private */
    void StreamWriter::sendHeaderFooter(const char* type) {
#ifdef DALHAL_API_STREAMWRITER_USE_SEPARATE_HEADER_FOOTER_BUFFER
#define DALHAL_API_STREAMWRITER_HEADER_FOOTER_BUFFER headerBuf
        char headerBuf[DALHAL_API_STREAMWRITER_HEADER_FOOTER_BUFFER_SIZE];
#else
#define DALHAL_API_STREAMWRITER_HEADER_FOOTER_BUFFER _buf
#endif
        int n = snprintf(DALHAL_API_STREAMWRITER_HEADER_FOOTER_BUFFER, sizeof(DALHAL_API_STREAMWRITER_HEADER_FOOTER_BUFFER),
                        "{\"type\":\"%s\",\"tag\":\"%s\",\"dataType\":\"%s\"}",
                        type, currentTag, GetCurrentType_cStr());

        ZeroCopyString zcSend(DALHAL_API_STREAMWRITER_HEADER_FOOTER_BUFFER, DALHAL_API_STREAMWRITER_HEADER_FOOTER_BUFFER+n);
        _cb(zcSend, CmdCbType::Text);
    }

    /* private */
    void StreamWriter::start(const char* tag, DataType dataType) {
        currentTag = tag;
        currentDataType = dataType;
        _pos = 0;
        const char* cStr = "{\"type\":\"start_chunked\"}";
        ZeroCopyString zcSend(cStr, sizeof("{\"type\":\"start_chunked\"}")-1);
        _cb(zcSend, CmdCbType::Text);
        //sendHeaderFooter("start_chunked");
    }

    /* public */
    void StreamWriter::restart(const char* tag, DataType dataType) {
        // end previous block
        end();
        // start new block
        start(tag, dataType);
    }

    /* public */
    void StreamWriter::write(const char* data, size_t len) {
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
    void StreamWriter::write(char c) {
        if (_pos >= sizeof(_buf)) flush();
        _buf[_pos++] = c;
    }

    /* private */
    void StreamWriter::end() {
        flush();
        sendHeaderFooter("end_chunked");
    }

    /* private */
    void StreamWriter::flush() {
        if (_pos == 0) return;
        ZeroCopyString zcSend(_buf, _pos);
        _cb(zcSend, CmdCbType::Binary);
        _pos = 0;
    }

}