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

#include "FS_Http.h"

#include <cstdio>
#include <cstring>
#include <string>

#include <HttpServer.h>
#include <LittleFS.h>
#include <Support/mimetable.h>
#include <Support/FS_Json.h>

#define FS_HTTP_MAX_PATH 128
#define FS_HTTP_MAX_FS_NAME 16

namespace FS_Http {

    using FS_Json::FsType;

    /** the filesystem of ?fs=, the error is sent when it is unknown or not mounted */
    static FsType* GetFs(httpd_req_t* req) {
        char name[FS_HTTP_MAX_FS_NAME] = "";
        HttpServer::GetQueryParam(req, "fs", name, sizeof(name)); // missing: "" = littlefs
        FsType* fs = FS_Json::Get(name);
        if (fs == nullptr) HttpServer::SendText(req, "404 Not Found", "filesystem not found or not mounted");
        return fs;
    }

    /** reads ?<key>= and checks it: absolute, no "..", not too long; the error is sent when not valid */
    static bool GetPath(httpd_req_t* req, char (&path)[FS_HTTP_MAX_PATH], const char* key = "path") {
        if (!HttpServer::GetQueryParam(req, key, path, sizeof(path))) {
            HttpServer::SendText(req, "400 Bad Request", "missing path parameter");
            return false;
        }
        if (!FS_Json::IsValidPath(path)) {
            HttpServer::SendText(req, "400 Bad Request", "path must start with / and must not contain ..");
            return false;
        }
        return true;
    }

    static esp_err_t SendJson(httpd_req_t* req, const std::string& json) {
        httpd_resp_set_type(req, "application/json");
        httpd_resp_set_hdr(req, "Cache-Control", "no-store");
        return httpd_resp_send(req, json.c_str(), json.length());
    }

    static esp_err_t MountsHandler(httpd_req_t* req) {
        std::string json;
        FS_Json::Mounts(json);
        return SendJson(req, json);
    }

    static esp_err_t ListHandler(httpd_req_t* req) {
        char path[FS_HTTP_MAX_PATH];
        FsType* fs = GetFs(req);
        if (fs == nullptr || !GetPath(req, path)) return ESP_OK;
        std::string json;
        if (!FS_Json::List(*fs, path, json)) return HttpServer::SendText(req, "404 Not Found", "folder not found");
        return SendJson(req, json);
    }

    static esp_err_t GetHandler(httpd_req_t* req) {
        char path[FS_HTTP_MAX_PATH];
        FsType* fs = GetFs(req);
        if (fs == nullptr || !GetPath(req, path)) return ESP_OK;
        File f = fs->open(path, "r");
        if (!f || f.isDirectory()) return HttpServer::SendText(req, "404 Not Found", "file not found");

        httpd_resp_set_type(req, mime::getContentType(path));
        char buf[1024];
        size_t n;
        while ((n = f.readBytes(buf, sizeof(buf))) > 0) {
            if (httpd_resp_send_chunk(req, buf, n) != ESP_OK) {
                httpd_resp_send_chunk(req, nullptr, 0);
                return ESP_FAIL;
            }
        }
        return httpd_resp_send_chunk(req, nullptr, 0);
    }

    static esp_err_t PutHandler(httpd_req_t* req) {
        char path[FS_HTTP_MAX_PATH];
        FsType* fs = GetFs(req);
        if (fs == nullptr || !GetPath(req, path)) return ESP_OK;
        const std::string tmpPath = std::string(path) + ".tmp";

        File f = fs->open(tmpPath.c_str(), "w");
        if (!f) return HttpServer::SendText(req, "500 Internal Server Error", "could not create the file (does the folder exist?)");

        char buf[1024];
        size_t remaining = req->content_len;
        while (remaining > 0) {
            int r = httpd_req_recv(req, buf, remaining < sizeof(buf) ? remaining : sizeof(buf));
            if (r == HTTPD_SOCK_ERR_TIMEOUT) continue; // slow client, try again
            if (r <= 0) {
                f.close();
                fs->remove(tmpPath.c_str());
                return ESP_FAIL; // connection lost, nothing to answer
            }
            if (f.write(reinterpret_cast<const uint8_t*>(buf), (size_t)r) != (size_t)r) {
                f.close();
                fs->remove(tmpPath.c_str());
                return HttpServer::SendText(req, "507 Insufficient Storage", "write failed (filesystem full?)");
            }
            remaining -= (size_t)r;
        }
        f.close();

        // replace the old file only now that the new one is complete
        if (fs->exists(path)) fs->remove(path);
        if (!fs->rename(tmpPath.c_str(), path)) {
            return HttpServer::SendText(req, "500 Internal Server Error", "rename failed, the upload is kept as .tmp");
        }
        printf("FS_Http: wrote %s (%u bytes)\r\n", path, (unsigned)req->content_len);
        return HttpServer::SendText(req, "200 OK", "OK");
    }

    static esp_err_t DeleteHandler(httpd_req_t* req) {
        char path[FS_HTTP_MAX_PATH];
        FsType* fs = GetFs(req);
        if (fs == nullptr || !GetPath(req, path)) return ESP_OK;
        if (strcmp(path, "/") == 0) return HttpServer::SendText(req, "400 Bad Request", "the root can not be deleted");
        File f = fs->open(path, "r");
        if (!f) return HttpServer::SendText(req, "404 Not Found", "file not found");
        const bool isDir = f.isDirectory();
        f.close();
        const bool ok = isDir ? fs->rmdir(path) : fs->remove(path);
        if (!ok) return HttpServer::SendText(req, "500 Internal Server Error", isDir ? "could not delete (folder not empty?)" : "could not delete");
        return HttpServer::SendText(req, "200 OK", "OK");
    }

    static esp_err_t MkdirHandler(httpd_req_t* req) {
        char path[FS_HTTP_MAX_PATH];
        FsType* fs = GetFs(req);
        if (fs == nullptr || !GetPath(req, path)) return ESP_OK;
        if (fs->exists(path)) return HttpServer::SendText(req, "409 Conflict", "already exists");
        if (!fs->mkdir(path)) return HttpServer::SendText(req, "500 Internal Server Error", "mkdir failed");
        return HttpServer::SendText(req, "200 OK", "OK");
    }

    static esp_err_t RenameHandler(httpd_req_t* req) {
        char from[FS_HTTP_MAX_PATH], to[FS_HTTP_MAX_PATH];
        FsType* fs = GetFs(req);
        if (fs == nullptr || !GetPath(req, from, "from") || !GetPath(req, to, "to")) return ESP_OK;
        if (!fs->exists(from)) return HttpServer::SendText(req, "404 Not Found", "file not found");
        if (fs->exists(to)) return HttpServer::SendText(req, "409 Conflict", "target already exists");
        if (!fs->rename(from, to)) return HttpServer::SendText(req, "500 Internal Server Error", "rename failed");
        return HttpServer::SendText(req, "200 OK", "OK");
    }

    void setup() {
        HttpServer::On("/fs/mounts", HTTP_GET, MountsHandler);
        HttpServer::On("/fs/list", HTTP_GET, ListHandler);
        HttpServer::On("/fs/file", HTTP_GET, GetHandler);
        HttpServer::On("/fs/file", HTTP_PUT, PutHandler);
        HttpServer::On("/fs/file", HTTP_DELETE, DeleteHandler);
        HttpServer::On("/fs/mkdir", HTTP_POST, MkdirHandler);
        HttpServer::On("/fs/rename", HTTP_POST, RenameHandler);
    }
}
