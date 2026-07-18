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

#include <DALHAL/Core/Types/DALHAL_ConstExpressionConstStrings.h>
#include <pgmspace.h>

namespace DALHAL {

    const char MIME_HTML[] PROGMEM = "text/html";
    const char MIME_JS[] PROGMEM = "application/javascript";
    const char MIME_CSS[] PROGMEM = "text/css";
    const char MIME_TEXT[] PROGMEM = "text/plain";

    const char MIME_JSON[] PROGMEM = "application/json";
    const char MIME_XML[] PROGMEM = "application/xml";

    const char MIME_PNG[] PROGMEM = "image/png";
    const char MIME_JPEG[] PROGMEM = "image/jpeg";
    const char MIME_GIF[] PROGMEM = "image/gif";
    const char MIME_SVG[] PROGMEM = "image/svg+xml";
    const char MIME_ICON[] PROGMEM = "image/x-icon";
    const char MIME_WEBP[] PROGMEM = "image/webp";

    const char MIME_BIN[] PROGMEM = "application/octet-stream";

    struct ApiVirtualFile {
        ConstExpressionStringComparableFn filename;
        /** progmem ptr */
        const uint8_t* data;
        size_t size;
        PGM_P mime;

        constexpr ApiVirtualFile(ConstExpressionStringComparableFn filename, const uint8_t* data, size_t size, PGM_P mime) : filename(filename), data(data), size(size), mime(mime) {}
    };
#define API_TEXT_FILE_ENTRY(filename, data, mime) \
    { CE_MATCH_EMIT_STR(filename), data, sizeof(data) - 1, mime }

#define API_BINARY_FILE_ENTRY(filename, data, mime) \
    { CE_MATCH_EMIT_STR(filename), data, sizeof(data), mime }
//#define API_VIRTUAL_FILE_ENTRY(filename, data, mime) {CE_MATCH_EMIT_STR(filename), data, sizeof(data)/sizeof(data[0]), mime}

}