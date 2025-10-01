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

#include "LittleFS_ext.h"
#include <LittleFS.h>
#include <Arduino.h>

namespace LittleFS_ext
{
    // --- Text loader (null-terminated, \n normalized) ---
    FileResult load_text_file(const char* file_name, char** outBuffer, size_t* outSize) {
        if (file_name == nullptr || strlen(file_name) == 0) {
            return FileResult::FileNameEmpty;
        }
        if (outBuffer == nullptr) {
            return FileResult::BufferPtrNull;
        }
        File this_file = LittleFS.open(file_name, "r");
        if (!this_file) {
            return FileResult::FileNotFound;
        }

        size_t size = this_file.size();
        if (size == 0) {
            this_file.close();
            return FileResult::FileEmpty;
        }

        char* buffer = new (std::nothrow) char[size + 1];
        if (!buffer) {
            this_file.close();
            return FileResult::AllocFail;
        }

        size_t readCount = this_file.readBytes(buffer, size);
        buffer[readCount] = '\0';

        // Normalize newlines in-place
        char* src = buffer;
        char* dst = buffer;
        while (*src) {
            if (*src == '\r') {
                if (src[1] == '\n') {
                    *dst++ = '\n';
                    src += 2;
                } else {
                    *dst++ = '\n';
                    src++;
                }
            } else {
                *dst++ = *src++;
            }
        }
        *dst = '\0';

        this_file.close();
        *outBuffer = buffer;
        if (outSize) *outSize = dst - buffer;
        return FileResult::Success;
    }

    // --- Binary loader (exact size, no modifications, no null terminator) ---
    FileResult load_binary_file(const char* file_name, uint8_t** outBuffer, size_t* outSize) {
        if (file_name == nullptr || strlen(file_name) == 0) {
            return FileResult::FileNameEmpty;
        }
        if (outBuffer == nullptr) {
            return FileResult::BufferPtrNull;
        }
        if (outSize == nullptr) {
            return FileResult::OutsizePtrNull;
        }
        File this_file = LittleFS.open(file_name, "r");
        if (!this_file) {
            return FileResult::FileNotFound;
        }

        size_t size = this_file.size();
        if (size == 0) {
            this_file.close();
            return FileResult::FileEmpty;
        }

        uint8_t* buffer = new (std::nothrow) uint8_t[size];
        if (!buffer) {
            this_file.close();
            return FileResult::AllocFail;
        }

        size_t readCount = this_file.readBytes(reinterpret_cast<char*>(buffer), size);
        this_file.close();

        *outBuffer = buffer;
        *outSize = readCount;

        return FileResult::Success;
    }

    int getFileSize(const char* file_name)
    {
        File this_file = LittleFS.open(file_name, "r");
        if (!this_file) { // failed to open the file, return empty result
            return -1;
        }
        int size = this_file.available();
        this_file.close();
        return size;
    }

    String GetNrSpaces(int count, bool isHtml) {
        String str;
        while (count-- > 0) {
            if (isHtml) str.concat("&nbsp;");
            else str.concat(" ");
        }
        return str;
    }
    void listDir(Stream &printStream, const char *dirname, uint8_t level) {
        printStream.printf(GetNrSpaces(level, false).c_str());
        printStream.printf("Listing directory: %s\r\n", dirname);
        level+=2;

        File root = LittleFS.open(dirname, "r");
        if (!root) {
            printStream.println(" - failed to open directory");
            return;
        }
        if (!root.isDirectory()) {
            printStream.println(" - not a directory");
            return;
        }

        File file = root.openNextFile();
        
        while (file) {
            if (file.isDirectory()) {
                printStream.printf(GetNrSpaces(level, false).c_str());
                printStream.print("DIR : ");
                printStream.println(file.name());
#if defined(ESP32)
                listDir(printStream, file.path(), level + 2);
#elif defined(ESP8266)
                listDir(printStream, file.fullName(), level + 2);
#endif
                
            } else {
                printStream.printf(GetNrSpaces(level, false).c_str());
                printStream.print("FILE: ");
                printStream.print(file.name());
                printStream.print("\tSIZE: ");
                printStream.println(file.size());
            }
            file = root.openNextFile();
        }
        printStream.println();
    }
    
    void listDir(String &str, bool isHtml, const char *dirname, uint8_t level) {
        str.concat(GetNrSpaces(level, isHtml));
        str.concat("Listing directory:"); str.concat(dirname);
        if (isHtml) str.concat("<br>"); else str.concat("\r\n");
        level+=2;

        File root = LittleFS.open(dirname, "r");
        if (!root) {
            str.concat(" - failed to open directory");
            if (isHtml) str.concat("<br>"); else str.concat("\r\n");
            return;
        }
        if (!root.isDirectory()) {
            str.concat(" - not a directory");
            if (isHtml) str.concat("<br>"); else str.concat("\r\n");
            return;
        }

        File file = root.openNextFile();
        while (file) {
            if (file.isDirectory()) {
                str.concat(GetNrSpaces(level, isHtml));
                str.concat("DIR : ");
                str.concat(file.name());
                if (isHtml) str.concat("<br>"); else str.concat("\r\n");
#if defined(ESP32)
                listDir(str, isHtml, file.path(), level + 2);
#elif defined(ESP8266)
                listDir(str, isHtml, file.fullName(), level + 2);
#endif
            } else {
                str.concat(GetNrSpaces(level, isHtml));
                str.concat("FILE: ");
                str.concat(file.name());
                str.concat("\tSIZE: ");
                str.concat(file.size());
                if (isHtml) str.concat("<br>"); else str.concat("\r\n");
            }
            file = root.openNextFile();
        }
        if (isHtml) str.concat("<br>"); else str.concat("\r\n");
    }
}
