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

#include <LittleFS.h>
#include "LittleFS_ext.h"

namespace LittleFS_ext
{
    
    FileResult load_from_file(const char* file_name, String &contents) {
        contents = "";
        
        
        File this_file = LittleFS.open(file_name, "r");
        if (!this_file) { // failed to open the file, retrn empty result
            return FileResult::FileNotFound;
        }
        contents.reserve(this_file.size()); // to minimize heap fragmentation

        while (this_file.available()) {
            char c = this_file.read();
            if (c == '\r') {
                // if next is '\n', skip next \r and only use \n
                if (this_file.peek() == '\n') {
                    this_file.read(); // remove \n from input
                }
                c = '\n'; // normalize
            }
            contents += c;
            //contents += (char)this_file.read();
        }
        
        this_file.close();
        return FileResult::Success;
    }

    /*FileResult load_from_file(const char* file_name, char *buff) {
        File this_file = LittleFS.open(file_name, "r");
        if (!this_file) { // failed to open the file, retrn empty result
            return FileResult::FileNotFound;
        }
        while (this_file.available()) {
            *buff = (char)this_file.read();
            buff++;
        }
        *buff = 0x00;
        this_file.close();
        return FileResult::Success;
    }*/
    FileResult load_from_file(const char* file_name, char** outBuffer, size_t* outSize) {
        File this_file = LittleFS.open(file_name, "r");
        if (!this_file) {
            return FileResult::FileNotFound;
        }

        size_t size = this_file.size();
        if (size == 0) {
            this_file.close();
            return FileResult::FileEmpty;
        }

        // Allocate buffer (+1 for null terminator)
        char* buffer = new (std::nothrow) char[size + 1];
        if (!buffer) {
            this_file.close();
            return FileResult::AllocFail;
        }

        // Read contents
        size_t index = 0;
        while (this_file.available()) {
            if (index >= size) {
                printf("\nERROR - load_from_file buffer overflow\n");
                delete[] buffer;
                return FileResult::BufferOverflowError;
            }
            char c = this_file.read();

            if (c == '\r') {
                // if next is '\n', skip next \r and only use \n
                if (this_file.peek() == '\n') {
                    this_file.read(); // remove \n from input
                }
                c = '\n'; // normalize
            }
            buffer[index++] = c;
           // buffer[index++] = this_file.read();
        }
        buffer[index] = '\0'; // Null-terminate

        this_file.close();

        *outBuffer = buffer;
        if (outSize) *outSize = index;
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
