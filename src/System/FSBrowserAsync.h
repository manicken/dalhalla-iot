/*
  FSBrowser - A web-based FileSystem Browser for ESP8266 filesystems

  Copyright (c) 2015 Hristo Gochkov. All rights reserved.
  This file is part of the ESP8266WebServer library for Arduino environment.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  See readme.md for more information.

  original at
  https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WebServer/examples/FSBrowser/FSBrowser.ino

  Modified by Jannik Svensson 2024 so that filesystem selection can be done
  Modified by Jannik Svensson 2025 to be used with AsyncWebServer
*/

#pragma once
#define FSBROWSER_ASYNC_WS_H_

#include <Arduino.h>
#include <SPI.h>
#include <LittleFS.h>
#if defined(ESP32) && !defined(seeed_xiao_esp32c3)
#include <SD_MMC.h>
#endif
//#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

namespace FSBrowser {

    
    FS *fileSystem = &LittleFS; // default FS
    bool fsOK = false;
    String unsupportedFiles = String();
    File uploadFile;

    const char TEXT_PLAIN[] PROGMEM = "text/plain";
    const char APPLICATION_JSON[] PROGMEM = "application/json";
    const char FS_INIT_ERROR[] PROGMEM = "FS INIT ERROR";
    const char FILE_NOT_FOUND[] PROGMEM = "FileNotFound";

    char upload_html[] PROGMEM = R"=====( 
<form method="post" enctype="multipart/form-data">
      <input type="file" name="name">
      <input class="button" type="submit" value="Upload">
      <br>
      <br>
      <button onclick="window.location.href='/edit'">Go to edit page</button>
</form>
)=====";

    const char* fsName = "LittleFS"; // main storage

    void replyOK(AsyncWebServerRequest *request) {
        request->send(200, FPSTR(TEXT_PLAIN), "");
    }

    void replyOKWithMsg(AsyncWebServerRequest *request, const String &msg) {
        request->send(200, FPSTR(TEXT_PLAIN), msg);
    }

    void replyNotFound(AsyncWebServerRequest *request, const String &msg) {
        request->send(404, FPSTR(TEXT_PLAIN), msg);
    }

    void replyBadRequest(AsyncWebServerRequest *request, const String &msg) {
        Serial.println(msg);
        request->send(400, FPSTR(TEXT_PLAIN), msg + "\r\n");
    }

    void replyServerError(AsyncWebServerRequest *request, const String &msg) {
        Serial.println(msg);
        request->send(500, FPSTR(TEXT_PLAIN), msg + "\r\n");
    }
    
    void handleFailsafeUploadPage(AsyncWebServerRequest *r) {
        r->send(200, "text/html", upload_html);
    }

    /*void handleFailsafeUploadPage(AsyncWebServerRequest *request) {
        String upload_html_str = (upload_html);
        AsyncWebServerResponse *response = request->beginResponse(
            200, "text/html", upload_html_str
        );
        response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
        response->addHeader("Pragma", "no-cache");
        response->addHeader("Expires", "-1");
        request->send(response);
    }*/
    File fsUploadFile;

    void handleFileUploadFailsafe(AsyncWebServerRequest *request,
                                const String &filename,
                                size_t index,
                                uint8_t *data,
                                size_t len,
                                bool final) {
        String dir = F("/edit");   // your upload directory
        String dest = F("/edit/upload"); // redirect after upload

        // Ensure the directory exists
        if (!LittleFS.exists(dir)) {
            LittleFS.mkdir(dir);
        }

        // Open file on first chunk
        if (index == 0) {
            String filePath = dir + "/" + filename;
            fsUploadFile = LittleFS.open(filePath, "w");
            if (!fsUploadFile) {
                Serial.println(F("Failed to open file for writing"));
                request->send(500, FPSTR(TEXT_PLAIN), F("500: couldn't create file"));
                return;
            }
            Serial.print(F("Upload Start: ")); Serial.println(filePath);
        }

        // Write chunk
        if (len && fsUploadFile) {
            fsUploadFile.write(data, len);
        }

        // Final chunk
        if (final) {
            if (fsUploadFile) {
                fsUploadFile.close();
                Serial.printf_P(PSTR("Upload End: %s, total bytes: %u\n"), filename.c_str(), index + len);

                // Redirect client
                AsyncWebServerResponse *response = request->beginResponse(303, "text/plain", "");
                response->addHeader("Location", dest);
                request->send(response);
            }
        }
    }

    bool selectFileSystemAndFixPath(String &path) {
#if defined(ESP32) && !defined(esp32c3)
        if (path.startsWith("/sdcard")) {
            fileSystem = &SD_MMC;
            path = path.substring(sizeof("/sdcard")-1);
            if (path.length() == 0) path = "/";
            return true;
        }
#endif
        if (path.startsWith(F("/LittleFS"))) {
            fileSystem = &LittleFS;
            path = path.substring(sizeof("/LittleFS")-1);
            if (path.length() == 0) path = '/';
            return true;
        }
        Serial.println(F("selectFileSystemAndFixPath error: invalid FS ")); Serial.println(path);
        return false;
    }

    void handleStatus(AsyncWebServerRequest *request) {
        String json = F("{\"type\":\""); json += String(fsName); json += F("\", \"isOk\":");
        if (fsOK) {
#if defined(ESP8266)
            FSInfo fs_info;
            LittleFS.info(fs_info);
            json += F("\"true\", \"totalBytes\":\"") + String(fs_info.totalBytes) + F("\", \"usedBytes\":\"") + String(fs_info.usedBytes) + '"';
#elif defined(ESP32)
            json += "\"true\", \"totalBytes\":\"" + String(LittleFS.totalBytes()) + "\", \"usedBytes\":\"" + String(LittleFS.usedBytes()) + "\"";
#endif
        } else {
            json += F("\"false\"");
        }
        json += F(",\"unsupportedFiles\":\""); json += unsupportedFiles; json += "\"}";
        request->send(200, FPSTR(APPLICATION_JSON), json);
    }

    void handleFileList(AsyncWebServerRequest *request) {
        if (!fsOK) return replyServerError(request, FPSTR(FS_INIT_ERROR));
        if (!request->hasParam("dir")) return replyBadRequest(request, F("DIR ARG MISSING"));

        String path = request->getParam("dir")->value();
        Serial.print(F("\nhandleFileList path:")); Serial.println(path.c_str());
        if (!selectFileSystemAndFixPath(path)) {
            request->send(200, FPSTR(APPLICATION_JSON),
                F("[{\"type\":\"dir\",\"name\":\"sdcard\"},{\"type\":\"dir\",\"name\":\"LittleFS\"}]"));
            return;
        }
        

        if (path != "/" && !fileSystem->exists(path)) return replyBadRequest(request, F("BAD PATH"));

        String output = "[";
    #if defined(ESP8266)
        Dir dir = fileSystem->openDir(path);
        while (dir.next()) {
            File f = dir.openFile("r");
            if (output.length() > 1) output += ",";
            output += F("{\"type\":\"");
            output += (f.isDirectory()) ? "dir" : "file";
            if (!f.isDirectory()) output += F("\",\"size\":\"") + String(f.size());
            output += F("\",\"name\":\"");
            output += (f.name()[0] == '/') ? &(f.name()[1]) : f.name();
            output += F("\"}");
            f.close();
        }
    #elif defined(ESP32)
        File dir = fileSystem->open(path);
        if (dir && dir.isDirectory()) {
            File f = dir.openNextFile();
            while (f) {
                if (output.length() > 1) output += ",";
                output += "{\"type\":\"";
                output += (f.isDirectory()) ? "dir" : "file";
                if (!f.isDirectory()) output += "\",\"size\":\"" + String(f.size());
                output += "\",\"name\":\"";
                String fname = f.name();
                if (fname[0] == '/') fname = fname.substring(1);
                output += fname;
                output += "\"}";
                f = dir.openNextFile();
            }
        }
    #endif
        output += "]";
        request->send(200, FPSTR(APPLICATION_JSON), output);
    }


    void handleFileRead(AsyncWebServerRequest *request) {
        String path = request->url();
        Serial.print(F("\nhandleFileRead path:")); Serial.println(path.c_str());
        if (!fsOK) { replyServerError(request, FPSTR(FS_INIT_ERROR)); return; }
        if (!selectFileSystemAndFixPath(path)) {
            IPAddress clientIP = request->client()->remoteIP();
            Serial.println(F("\r\n=== DEBUG: /json request ==="));
            Serial.print(F("From IP: "));
            Serial.println(clientIP.toString());
            Serial.print(F("Timestamp: "));
            Serial.println(millis());
            
            // Logga HTTP headers
            Serial.println(F("Headers:"));
            for (int i = 0; i < request->headers(); i++) {
                AsyncWebHeader* h = request->getHeader(i);
                Serial.print("  ");
                Serial.print(h->name());
                Serial.print(": ");
                Serial.println(h->value());
            }
            replyNotFound(request, F("FS NOT FOUND"));
            return;
        }
        if (!fileSystem->exists(path)) { replyNotFound(request, FPSTR(FILE_NOT_FOUND)); return; }
        
        AsyncWebServerResponse *response = request->beginResponse(
            fileSystem->open(path, "r"), "application/octet-stream");
        request->send(response);
    }

    void handleFileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
        if (!fsOK) return;
        if (index == 0) {
            
            Serial.print(F("\nhandleFileUpload:")); Serial.println(filename.c_str());
            if (!filename.startsWith("/")) filename = "/" + filename;
            if (!selectFileSystemAndFixPath(filename)) { replyNotFound(request, F("FS NOT FOUND")); return; }
            uploadFile = fileSystem->open(filename, "w");
            if (!uploadFile) { request->send(500, FPSTR(TEXT_PLAIN), F("CREATE FAILED")); return; }
        }
        if (len && uploadFile) uploadFile.write(data, len);
        if (final && uploadFile) uploadFile.close();
    }

    void handleFileCreate(AsyncWebServerRequest *request) {
        if (!fsOK) return replyServerError(request, FPSTR(FS_INIT_ERROR));
        if (!request->hasParam("path", true)) return replyBadRequest(request, F("PATH ARG MISSING"));

        String path = request->getParam("path", true)->value();
        Serial.print(F("\nhandleFileCreate path:")); Serial.println(path.c_str());

        if (!selectFileSystemAndFixPath(path)) return;
        
        
        if (path == "/" || fileSystem->exists(path)) return replyBadRequest(request, F("BAD PATH"));
        String src = request->arg("src");
        if (src.isEmpty()) {
            // No source specified: creation
            Serial.print(F("\nhandleFileCreate path:")); Serial.println(path.c_str());
            if (path.endsWith("/")) {
                // Create a folder
                path.remove(path.length() - 1);
                if (!fileSystem->mkdir(path)) { return replyServerError(request, F("MKDIR FAILED")); }
                replyOKWithMsg(request, path);
            } else {
            
                File file = fileSystem->open(path, "w");
                if (!file) return replyServerError(request, F("CREATE FAILED"));
                file.close();
                replyOKWithMsg(request, path.substring(0, path.lastIndexOf('/')));
            }
        } else {
            // Source specified: rename
            if (src == "/") { return replyBadRequest(request, F("BAD SRC")); }
            if (!fileSystem->exists(src)) { return replyBadRequest(request, F("SRC FILE NOT FOUND")); }

            Serial.print(F("\nhandleFileCreate:")); Serial.print(path.c_str()); Serial.print(F(" from ")); Serial.println(src.c_str());

            if (path.endsWith("/")) { path.remove(path.length() - 1); }
            if (src.endsWith("/")) { src.remove(src.length() - 1); }
            if (!fileSystem->rename(src, path)) { return replyServerError(request, F("RENAME FAILED")); }
            replyOKWithMsg(request, src.substring(0, src.lastIndexOf('/')));
        }
    }

    void handleFileDelete(AsyncWebServerRequest *request) {
        if (!fsOK) return replyServerError(request, FPSTR(FS_INIT_ERROR));
        if (!request->hasParam("path", true)) return replyBadRequest(request, F("PATH ARG MISSING"));

        String path = request->getParam("path", true)->value();
        Serial.print(F("\nhandleFileDelete path:")); Serial.println(path.c_str());
        if (!selectFileSystemAndFixPath(path)) return;
        if (path.isEmpty() || path == "/") return replyBadRequest(request, F("BAD PATH"));
        if (!fileSystem->exists(path)) return replyNotFound(request, FPSTR(FILE_NOT_FOUND));
        
        fileSystem->remove(path);
        replyOKWithMsg(request, path.substring(0, path.lastIndexOf('/')));
    }

    void setup(AsyncWebServer &srv) {
        
        fsOK = LittleFS.begin();

        srv.on("/status", HTTP_GET, handleStatus);
        srv.on("/list", HTTP_GET, handleFileList);
        srv.on("/edit/upload", HTTP_GET, handleFailsafeUploadPage);           // if the client requests the upload page
        srv.on("/edit/upload", HTTP_POST, [](AsyncWebServerRequest *r){ Serial.println(F("send OK")); r->send(200); }, handleFileUploadFailsafe);

        srv.on("/edit", HTTP_GET, [](AsyncWebServerRequest *r){ r->send(LittleFS, "/edit/index.htm", "text/html"); });
        srv.on("/edit", HTTP_PUT, handleFileCreate);
        srv.on("/edit", HTTP_DELETE, handleFileDelete);
        srv.on("/upload", HTTP_GET, handleFailsafeUploadPage);
        srv.on("/upload", HTTP_POST, [](AsyncWebServerRequest *r){ Serial.println(F("send OK")); r->send(200); }, handleFileUploadFailsafe);
        srv.on("/edit", HTTP_POST, [](AsyncWebServerRequest *r){ r->send(200); }, handleFileUpload);

        srv.serveStatic("/", LittleFS, "/").setDefaultFile("index.htm");
        srv.onNotFound(handleFileRead);
    }

}
