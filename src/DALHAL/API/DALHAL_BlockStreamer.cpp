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

#include "DALHAL_BlockStreamer.h"

#define DALHAL_API_STREAMWRITER_HEADER_FOOTER_BUFFER_SIZE 128

namespace DALHAL {

    BlockStreamer::BlockStreamer(CommandCallback cb, const char* tag, DataType dataType) :
        _cb(cb), 
        // lambda wrapper 
        sbs([cb](const char* buff, size_t len) -> bool {
            ZeroCopyString zcData(buff, len);
            return cb(zcData, CmdCbType::Data);
        })
    {
        start(tag, dataType);
    }

    BlockStreamer::~BlockStreamer() {
        end();
    }

    /* private */
    const char* BlockStreamer::GetCurrentType_cStr() {
        if (currentDataType == DataType::Json) {
            return "json";
        } else if (currentDataType == DataType::PlainText) {
            return "text";
        } else {
            return "unknown";
        }
    }

    /* private */
    void BlockStreamer::sendHeaderFooter(const char* type) {

        char headerBuf[DALHAL_API_STREAMWRITER_HEADER_FOOTER_BUFFER_SIZE];

        int n = snprintf(headerBuf, sizeof(headerBuf),
                        "{\"type\":\"%s\",\"tag\":\"%s\",\"dataType\":\"%s\"}",
                        type, currentTag, GetCurrentType_cStr());

        ZeroCopyString zcSend(headerBuf, n);
        _cb(zcSend, CmdCbType::Control);
    }

    /* private */
    void BlockStreamer::start(const char* tag, DataType dataType) {
        currentTag = tag;
        currentDataType = dataType;

        const char* cStr = "{\"type\":\"start_chunked\"}";
        ZeroCopyString zcSend(cStr, sizeof("{\"type\":\"start_chunked\"}")-1);
        _cb(zcSend, CmdCbType::Control);
        // sending the data like this make it possible in the future to send tree structures of data
        // but only if the receiver can handle it,
        // i.e. sending one different block of data inside annother that is handled separately from the main one 
        //sendHeaderFooter("start_chunked");
    }

    /* public */
    void BlockStreamer::restart(const char* tag, DataType dataType) {
        // end previous block
        end();
        // start new block
        start(tag, dataType);
    }

    /* private */
    void BlockStreamer::end() {
        sbs.flush();
        sendHeaderFooter("end_chunked");
    }


    

}