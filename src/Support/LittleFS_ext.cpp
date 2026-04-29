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

#if defined(ESP32) || defined(ESP8266)

#include "LittleFS_ext.h"
#include <LittleFS.h>
#include <Arduino.h>
#include <string>

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

        char* buffer = new char[size + 1];
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

        uint8_t* buffer = new uint8_t[size];
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

    std::string GetNrSpaces(int count, bool isHtml) {
        std::string str;
        while (count-- > 0) {
            if (isHtml) str.append("&nbsp;");
            else str.append(" ");
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
    
    

    void listDir(std::string &str, ListMode mode, const char *dirname, uint8_t level/* = 0*/) {
        // Indentation helper
        auto indent = [&](uint8_t l) -> std::string {
            std::string s;
            for (uint8_t i = 0; i < l; i++) s += (mode == ListMode::HTML ? "&nbsp;" : " ");
            return s;
        };

        if (mode == ListMode::JSON) {
            str.append("{");
            str.append("\"dir\":\"");
            str.append(dirname);
            str.append("\",\"entries\":[");
        } else {
            str.append(indent(level));
            str.append("Listing directory: ");
            str.append(dirname);
            str.append(mode == ListMode::HTML ? "<br>" : "\r\n");
        }

        File root = LittleFS.open(dirname, "r");
        if (!root) {
            str.append(indent(level + 2));
            str.append(" - failed to open directory");
            str.append(mode == ListMode::HTML ? "<br>" : "\r\n");
            if (mode == ListMode::JSON) str.append("]");
            return;
        }
        if (!root.isDirectory()) {
            str.append(indent(level + 2));
            str.append(" - not a directory");
            str.append(mode == ListMode::HTML ? "<br>" : "\r\n");
            if (mode == ListMode::JSON) str.append("]");
            return;
        }

        File file = root.openNextFile();
        bool firstEntry = true;
        while (file) {
            if (mode == ListMode::JSON && !firstEntry) str.append(",");
            firstEntry = false;

            if (file.isDirectory()) {
                if (mode == ListMode::JSON) {
#if defined(ESP32)
                    listDir(str, mode, file.path(), level + 2);
#elif defined(ESP8266)
                    listDir(str, mode, file.fullName(), level + 2);
#endif
                } else {
                    str.append(indent(level + 2));
                    str.append("DIR : ");
                    str.append(file.name());
                    str.append(mode == ListMode::HTML ? "<br>" : "\r\n");
#if defined(ESP32)
                    listDir(str, mode, file.path(), level + 2);
#elif defined(ESP8266)
                    listDir(str, mode, file.fullName(), level + 2);
#endif
                }
            } else {
                if (mode == ListMode::JSON) {
                    str.append("{\"file\":\"");
                    str.append(file.name());
                    str.append("\",\"size\":");
                    str.append(std::to_string(file.size()));
                    str.append("}");
                } else {
                    str.append(indent(level + 2));
                    str.append("FILE: ");
                    str.append(file.name());
                    str.append("\tSIZE: ");
                    str.append(std::to_string(file.size()));
                    str.append(mode == ListMode::HTML ? "<br>" : "\r\n");
                }
            }
            file = root.openNextFile();
        }

        if (mode == ListMode::JSON) str.append("]}");
        else str.append(mode == ListMode::HTML ? "<br>" : "\r\n");
    }
}

#endif