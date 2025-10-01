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

#include <Arduino.h>

namespace LittleFS_ext
{
    enum class FileResult {
        Success,
        FileNameEmpty,
        OutsizePtrNull,
        BufferPtrNull,
        FileNotFound,
        FileEmpty,
        AllocFail,
        FileReadError,
        BufferOverflowError
    };
    /** --- Text loader (null-terminated, \n normalized) --- */
    FileResult load_text_file(const char* file_name, char** outBuffer, size_t* outSize = nullptr);
    /** --- Binary loader (exact size, no modifications, no null terminator) --- */
    FileResult load_binary_file(const char* file_name, uint8_t** outBuffer, size_t* outSize);

    int getFileSize(const char* file_name);
    void listDir(Stream &printStream, const char *dirname, uint8_t level);
    void listDir(String &str, bool isHtml, const char *dirname, uint8_t level);
}
