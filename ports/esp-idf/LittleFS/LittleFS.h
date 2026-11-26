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

/**
  The part of the Arduino FS API (LittleFS, File) that DALHAL uses, on ESP-IDF.
  Mounted with esp_littlefs on the VFS at /littlefs, partition "spiffs", the same as arduino-esp32,
  so partition tables and filesystem images are shared with the Arduino builds.
  Paths given to this API are relative to the mount point ("/hal/cfg.json"), as on the Arduino core.
*/

#pragma once

#include <Stream.h>
#include <cstdio>
#include <string>
#include <dirent.h>

class File : public Stream {
public:
    File() = default;
    File(const File&) = delete;
    File& operator=(const File&) = delete;
    File(File&& other) noexcept { *this = std::move(other); }
    File& operator=(File&& other) noexcept;
    ~File() override { close(); }

    explicit operator bool() const { return fp != nullptr || dir != nullptr; }

    size_t size() const;
    size_t position() const;
    bool seek(size_t pos);
    void close();

    int available() override;
    int read() override;
    int peek() override;
    size_t readBytes(char* buffer, size_t length);
    size_t read(uint8_t* buffer, size_t length) { return readBytes(reinterpret_cast<char*>(buffer), length); }
    size_t write(uint8_t b) override;
    size_t write(const uint8_t* buffer, size_t size) override;
    using Print::write;
    void flush() override;

    bool isDirectory() const { return dir != nullptr; }
    /** name without the folder, as arduino-esp32 */
    const char* name() const;
    /** full path relative to the mount point */
    const char* path() const { return relPath.c_str(); }
    /** next entry when this is an opened directory, an invalid File when there are no more */
    File openNextFile();

private:
    friend class LittleFSFS;
    FILE* fp = nullptr;
    DIR* dir = nullptr;
    std::string relPath;
};

class LittleFSFS {
public:
    /** mounts the filesystem, basePath is where it shows up on the VFS */
    bool begin(bool formatOnFail = false, const char* basePath = "/littlefs", uint8_t maxOpenFiles = 10, const char* partitionLabel = "spiffs");
    void end();
    bool format();

    File open(const char* path, const char* mode = "r");
    bool exists(const char* path);
    bool mkdir(const char* path);
    bool rmdir(const char* path);
    bool remove(const char* path);
    bool rename(const char* from, const char* to);

    size_t totalBytes();
    size_t usedBytes();

    /** the VFS path for a path relative to the mount point, for code that uses stdio directly */
    std::string vfsPath(const char* path) const;

private:
    std::string base = "/littlefs";
    std::string label = "spiffs";
    bool mounted = false;
};

extern LittleFSFS LittleFS;
