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

#ifndef LITTLEFS_EXT_H
#define LITTLEFS_EXT_H

#include <LittleFS.h>
namespace LittleFS_ext
{
    enum class FileResult {
        Success,
        FileNotFound,
        FileEmpty,
        AllocFail,
        FileReadError,
        BufferOverflowError
    };
    /** note this file reader normalizes \r,\r\n to simple \n  */
    FileResult load_from_file(const char* file_name, String &contents);
    /** please note that using this function is not safe,
     * and buff size must be bigger or equal to filesize + 1
     * as otherwise it will overwrite data outside buff
     * and this will likely corrupt data to lead to undefined
     * behaviour
     */
    //FileResult load_from_file(const char* file_name, char *buff);
    /** using this is safe, note this file reader normalizes \r,\r\n to simple \n */
    FileResult load_from_file(const char* file_name, char** outBuffer, size_t* outSize = nullptr);
    int getFileSize(const char* file_name);
    void listDir(Stream &printStream, const char *dirname, uint8_t level);
    void listDir(String &str, bool isHtml, const char *dirname, uint8_t level);
}

#endif