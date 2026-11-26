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

#include "LittleFS.h"

#include <cstring>
#include <sys/stat.h>
#include <unistd.h>
#include "esp_littlefs.h"

LittleFSFS LittleFS;

// ---------------- File ----------------

File& File::operator=(File&& other) noexcept {
    if (this != &other) {
        close();
        fp = other.fp; dir = other.dir; relPath = std::move(other.relPath);
        other.fp = nullptr; other.dir = nullptr;
    }
    return *this;
}

void File::close() {
    if (fp) { fclose(fp); fp = nullptr; }
    if (dir) { closedir(dir); dir = nullptr; }
}

size_t File::size() const {
    if (!fp) return 0;
    struct stat st;
    if (fstat(fileno(fp), &st) != 0) return 0;
    return (size_t)st.st_size;
}

size_t File::position() const {
    if (!fp) return 0;
    long pos = ftell(fp);
    return pos < 0 ? 0 : (size_t)pos;
}

bool File::seek(size_t pos) {
    return fp && fseek(fp, (long)pos, SEEK_SET) == 0;
}

int File::available() {
    if (!fp) return 0;
    size_t sz = size(), pos = position();
    return pos < sz ? (int)(sz - pos) : 0;
}

int File::read() {
    if (!fp) return -1;
    int c = fgetc(fp);
    return c == EOF ? -1 : c;
}

int File::peek() {
    if (!fp) return -1;
    int c = fgetc(fp);
    if (c == EOF) return -1;
    ungetc(c, fp);
    return c;
}

size_t File::readBytes(char* buffer, size_t length) {
    if (!fp || !buffer) return 0;
    return fread(buffer, 1, length, fp);
}

size_t File::write(uint8_t b) {
    return fp && fputc(b, fp) != EOF ? 1 : 0;
}

size_t File::write(const uint8_t* buffer, size_t size) {
    return fp ? fwrite(buffer, 1, size, fp) : 0;
}

void File::flush() {
    if (fp) fflush(fp);
}

const char* File::name() const {
    const char* slash = strrchr(relPath.c_str(), '/');
    return slash ? slash + 1 : relPath.c_str();
}

File File::openNextFile() {
    File f;
    if (!dir) return f;
    struct dirent* entry = readdir(dir);
    if (!entry) return f;
    std::string childRel = relPath;
    if (childRel.empty() || childRel.back() != '/') childRel += '/';
    childRel += entry->d_name;
    f = LittleFS.open(childRel.c_str(), "r");
    return f;
}

// ---------------- LittleFSFS ----------------

std::string LittleFSFS::vfsPath(const char* path) const {
    std::string full = base;
    if (path == nullptr || path[0] != '/') full += '/';
    if (path) full += path;
    return full;
}

bool LittleFSFS::begin(bool formatOnFail, const char* basePath, uint8_t maxOpenFiles, const char* partitionLabel) {
    (void)maxOpenFiles; // esp_littlefs has no open file limit
    if (mounted) return true;
    base = basePath ? basePath : "/littlefs";
    label = partitionLabel ? partitionLabel : "spiffs";

    esp_vfs_littlefs_conf_t conf = {};
    conf.base_path = base.c_str();
    conf.partition_label = label.c_str();
    conf.format_if_mount_failed = formatOnFail;
    conf.dont_mount = false;
    mounted = esp_vfs_littlefs_register(&conf) == ESP_OK;
    return mounted;
}

void LittleFSFS::end() {
    if (!mounted) return;
    esp_vfs_littlefs_unregister(label.c_str());
    mounted = false;
}

bool LittleFSFS::format() {
    return esp_littlefs_format(label.c_str()) == ESP_OK;
}

File LittleFSFS::open(const char* path, const char* mode) {
    File f;
    if (!mounted || path == nullptr) return f;
    std::string full = vfsPath(path);
    f.relPath = path;

    struct stat st;
    if (strchr(mode, 'r') && strchr(mode, '+') == nullptr && stat(full.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
        f.dir = opendir(full.c_str());
        return f;
    }
    f.fp = fopen(full.c_str(), mode);
    return f;
}

bool LittleFSFS::exists(const char* path) {
    if (!mounted || path == nullptr) return false;
    struct stat st;
    return stat(vfsPath(path).c_str(), &st) == 0;
}

bool LittleFSFS::mkdir(const char* path) {
    return mounted && path && ::mkdir(vfsPath(path).c_str(), 0775) == 0;
}

bool LittleFSFS::rmdir(const char* path) {
    return mounted && path && ::rmdir(vfsPath(path).c_str()) == 0;
}

bool LittleFSFS::remove(const char* path) {
    return mounted && path && ::unlink(vfsPath(path).c_str()) == 0;
}

bool LittleFSFS::rename(const char* from, const char* to) {
    return mounted && from && to && ::rename(vfsPath(from).c_str(), vfsPath(to).c_str()) == 0;
}

size_t LittleFSFS::totalBytes() {
    size_t total = 0, used = 0;
    esp_littlefs_info(label.c_str(), &total, &used);
    return total;
}

size_t LittleFSFS::usedBytes() {
    size_t total = 0, used = 0;
    esp_littlefs_info(label.c_str(), &total, &used);
    return used;
}
