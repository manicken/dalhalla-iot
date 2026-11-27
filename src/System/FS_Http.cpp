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

#if defined(DALHAL_BACKEND_ARDUINO)

#include <string>
#include <Support/FS_Json.h>

namespace FS_Http {

    using FS_Json::FsType;

    static const char TEXT_PLAIN[] PROGMEM = "text/plain";
    static const char APPLICATION_JSON[] PROGMEM = "application/json";

    static void SendText(AsyncWebServerRequest* request, int code, const __FlashStringHelper* text) {
        request->send(code, FPSTR(TEXT_PLAIN), text);
    }

    /** the filesystem of ?fs=, the error is sent when it is unknown or not mounted */
    static FsType* GetFs(AsyncWebServerRequest* request) {
        FsType* fs = FS_Json::Get(request->hasParam("fs") ? request->getParam("fs")->value().c_str() : nullptr);
        if (fs == nullptr) SendText(request, 404, F("filesystem not found or not mounted"));
        return fs;
    }

    /** the value of ?<key>=, empty when missing or not a valid path (the error is then already sent) */
    static String GetPath(AsyncWebServerRequest* request, const char* key = "path") {
        if (!request->hasParam(key)) { SendText(request, 400, F("missing path parameter")); return String(); }
        String path = request->getParam(key)->value();
        if (!FS_Json::IsValidPath(path.c_str())) {
            SendText(request, 400, F("path must start with / and must not contain .."));
            return String();
        }
        return path;
    }

    static void SendJson(AsyncWebServerRequest* request, const std::string& json) {
        AsyncWebServerResponse* response = request->beginResponse(200, FPSTR(APPLICATION_JSON), json.c_str());
        response->addHeader(F("Cache-Control"), F("no-store"));
        request->send(response);
    }

    static void HandleMounts(AsyncWebServerRequest* request) {
        std::string json;
        FS_Json::Mounts(json);
        SendJson(request, json);
    }

    static void HandleList(AsyncWebServerRequest* request) {
        FsType* fs = GetFs(request);
        if (fs == nullptr) return;
        String path = GetPath(request);
        if (path.length() == 0) return;
        std::string json;
        if (!FS_Json::List(*fs, path.c_str(), json)) { SendText(request, 404, F("folder not found")); return; }
        SendJson(request, json);
    }

    static void HandleGet(AsyncWebServerRequest* request) {
        FsType* fs = GetFs(request);
        if (fs == nullptr) return;
        String path = GetPath(request);
        if (path.length() == 0) return;
        File f = fs->open(path, "r");
        const bool isFile = f && !f.isDirectory();
        f.close();
        if (!isFile) { SendText(request, 404, F("file not found")); return; }
        request->send(*fs, path); // content type from the extension
    }

    // one upload at a time: the body arrives in parts (HandlePutBody), then HandlePut answers;
    // it is written to <path>.tmp and renamed when complete
    static File putFile;
    static String putPath;
    static FsType* putFs = nullptr;
    static bool putFailed = false;

    static void HandlePutBody(AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
        if (index == 0) {
            putFailed = false;
            if (putFile) putFile.close(); // a previous broken upload
            putFs = FS_Json::Get(request->hasParam("fs") ? request->getParam("fs")->value().c_str() : nullptr);
            putPath = request->hasParam("path") ? request->getParam("path")->value() : String();
            if (putFs == nullptr || !FS_Json::IsValidPath(putPath.c_str())) { putFailed = true; return; }
            putFile = putFs->open(putPath + ".tmp", "w");
            if (!putFile) { putFailed = true; return; }
        }
        if (putFailed || !putFile) return;
        if (putFile.write(data, len) != len) {
            putFailed = true;
            putFile.close();
            putFs->remove(putPath + ".tmp");
            return;
        }
        if (index + len == total) putFile.close();
    }

    static void HandlePut(AsyncWebServerRequest* request) { // called when the whole body is received
        FsType* fs = GetFs(request);
        if (fs == nullptr) return;
        String path = GetPath(request);
        if (path.length() == 0) return;
        if (request->contentLength() == 0) { // no body: an empty file
            File f = fs->open(path, "w");
            const bool ok = (bool)f;
            f.close();
            if (ok) SendText(request, 200, F("OK"));
            else SendText(request, 500, F("could not create the file (does the folder exist?)"));
            return;
        }
        if (putFailed || fs != putFs || path != putPath) {
            if (putFile) putFile.close();
            fs->remove(path + ".tmp");
            SendText(request, 500, F("write failed (does the folder exist? filesystem full?)"));
            return;
        }
        if (putFile) putFile.close();
        // replace the old file only now that the new one is complete
        if (fs->exists(path)) fs->remove(path);
        if (!fs->rename(path + ".tmp", path)) { SendText(request, 500, F("rename failed, the upload is kept as .tmp")); return; }
        SendText(request, 200, F("OK"));
    }

    static void HandleDelete(AsyncWebServerRequest* request) {
        FsType* fs = GetFs(request);
        if (fs == nullptr) return;
        String path = GetPath(request);
        if (path.length() == 0) return;
        if (path == "/") { SendText(request, 400, F("the root can not be deleted")); return; }
        File f = fs->open(path, "r");
        if (!f) { SendText(request, 404, F("file not found")); return; }
        const bool isDir = f.isDirectory();
        f.close();
        const bool ok = isDir ? fs->rmdir(path) : fs->remove(path);
        if (ok) SendText(request, 200, F("OK"));
        else if (isDir) SendText(request, 500, F("could not delete (folder not empty?)"));
        else SendText(request, 500, F("could not delete"));
    }

    static void HandleMkdir(AsyncWebServerRequest* request) {
        FsType* fs = GetFs(request);
        if (fs == nullptr) return;
        String path = GetPath(request);
        if (path.length() == 0) return;
        if (fs->exists(path)) { SendText(request, 409, F("already exists")); return; }
        if (!fs->mkdir(path)) { SendText(request, 500, F("mkdir failed")); return; }
        SendText(request, 200, F("OK"));
    }

    static void HandleRename(AsyncWebServerRequest* request) {
        FsType* fs = GetFs(request);
        if (fs == nullptr) return;
        String from = GetPath(request, "from");
        if (from.length() == 0) return;
        String to = GetPath(request, "to");
        if (to.length() == 0) return;
        if (!fs->exists(from)) { SendText(request, 404, F("file not found")); return; }
        if (fs->exists(to)) { SendText(request, 409, F("target already exists")); return; }
        if (!fs->rename(from, to)) { SendText(request, 500, F("rename failed")); return; }
        SendText(request, 200, F("OK"));
    }

    void setup(AsyncWebServer& server) {
        server.on("/fs/mounts", HTTP_GET, HandleMounts);
        server.on("/fs/list", HTTP_GET, HandleList);
        server.on("/fs/file", HTTP_GET, HandleGet);
        server.on("/fs/file", HTTP_PUT, HandlePut, nullptr, HandlePutBody);
        server.on("/fs/file", HTTP_DELETE, HandleDelete);
        server.on("/fs/mkdir", HTTP_POST, HandleMkdir);
        server.on("/fs/rename", HTTP_POST, HandleRename);
    }
}

#endif
