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

#include "FS_Json.h"

#if defined(DALHAL_PLATFORM_ESP32) || defined(DALHAL_PLATFORM_ESP8266)

#include <cstring>
#include <cstdint>
#include <LittleFS.h>
#if defined(FS_JSON_HAS_SD_MMC)
#include <SD_MMC.h>
#endif

#define FS_JSON_MAX_PATH 128

namespace FS_Json {

    // ---------------- the filesystems ----------------

    struct Mount {
        const char* name;
        FsType* fs;
        bool (*mounted)();
        void (*usage)(uint32_t& total, uint32_t& used);
    };

    static void LittleFsUsage(uint32_t& total, uint32_t& used) {
#if defined(DALHAL_PLATFORM_ESP8266)
        FSInfo info;
        if (LittleFS.info(info)) { total = (uint32_t)info.totalBytes; used = (uint32_t)info.usedBytes; }
#else
        total = (uint32_t)LittleFS.totalBytes();
        used = (uint32_t)LittleFS.usedBytes();
#endif
    }

    static const Mount mounts[] = {
        { "littlefs", &LittleFS,
          []() { uint32_t t = 0, u = 0; LittleFsUsage(t, u); return t > 0; },
          LittleFsUsage },
#if defined(FS_JSON_HAS_SD_MMC)
        { "sd", &SD_MMC,
          []() { return SD_MMC.cardType() != CARD_NONE; }, // CARD_NONE also when SD_MMC.begin() was never called
          [](uint32_t& total, uint32_t& used) { total = (uint32_t)SD_MMC.totalBytes(); used = (uint32_t)SD_MMC.usedBytes(); } },
#endif
    };

    FsType* Get(const char* name) {
        if (name == nullptr || *name == '\0') name = "littlefs";
        for (const Mount& m : mounts) {
            if (strcmp(m.name, name) == 0) return m.mounted() ? m.fs : nullptr;
        }
        return nullptr;
    }

    void Mounts(std::string& out) {
        out = "[";
        for (const Mount& m : mounts) {
            if (!m.mounted()) continue;
            uint32_t total = 0, used = 0;
            m.usage(total, used);
            if (out.size() > 1) out += ',';
            out += "{\"fs\":\"";
            out += m.name;
            out += "\",\"total\":";
            out += std::to_string(total);
            out += ",\"used\":";
            out += std::to_string(used);
            out += '}';
        }
        out += ']';
    }

    // ---------------- paths and listing ----------------

    bool IsValidPath(const char* path) {
        return path != nullptr && path[0] == '/' && strstr(path, "..") == nullptr && strlen(path) < FS_JSON_MAX_PATH;
    }

    static void AppendQuoted(std::string& out, const char* s) {
        out += '"';
        for (; *s; s++) {
            if (*s == '"' || *s == '\\') out += '\\';
            out += *s;
        }
        out += '"';
    }

    static void AppendEntry(std::string& out, const char* name, bool isDir, uint32_t size) {
        if (out.size() > 1) out += ',';
        out += "{\"name\":";
        if (name[0] == '/') name++; // some cores give the name with a leading /
        AppendQuoted(out, name);
        out += isDir ? ",\"dir\":true,\"size\":0}" : ",\"dir\":false,\"size\":";
        if (!isDir) {
            out += std::to_string(size);
            out += '}';
        }
    }

    bool List(FsType& fs, const char* path, std::string& out) {
        out = "[";
#if defined(DALHAL_PLATFORM_ESP8266)
        File d = fs.open(path, "r");
        const bool isDir = strcmp(path, "/") == 0 || (d && d.isDirectory());
        d.close();
        if (!isDir) return false;
        Dir dir = fs.openDir(path);
        while (dir.next()) {
            AppendEntry(out, dir.fileName().c_str(), dir.isDirectory(), (uint32_t)dir.fileSize());
        }
#else // ESP32: arduino-esp32 and the ESP-IDF port have the same File API
        File dir = fs.open(path, "r");
        if (!dir || !dir.isDirectory()) return false;
        File f = dir.openNextFile();
        while (f) {
            AppendEntry(out, f.name(), f.isDirectory(), (uint32_t)f.size());
            f = dir.openNextFile();
        }
#endif
        out += ']';
        return true;
    }
}

#endif
