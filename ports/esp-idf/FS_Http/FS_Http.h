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
  The filesystems over HTTP, ESP-IDF backend (on the HttpServer, port 80). The same routes and answers
  as the Arduino version (src/System/FS_Http), used by the Files tab of the frontend:
    GET    /fs/mounts                          [{"fs":"littlefs","total":..,"used":..}, ...]
    GET    /fs/list?fs=&path=/dir              folder content as JSON (FS_Json::List)
    GET    /fs/file?fs=&path=/dir/name         download
    PUT    /fs/file?fs=&path=/dir/name         upload, the request body is the file content
    DELETE /fs/file?fs=&path=/dir/name         delete a file or an empty folder
    POST   /fs/mkdir?fs=&path=/dir             create a folder
    POST   /fs/rename?fs=&from=..&to=..        rename/move within one filesystem
  fs= selects the filesystem ("littlefs" when missing), see FS_Json::Get.
  An upload is written to <path>.tmp first and replaces the file only when it is complete,
  so a broken upload never leaves a half written cfg.json.
*/

#pragma once

namespace FS_Http {
    /** adds the /fs/... routes, call after HttpServer::Start() */
    void setup();
}
