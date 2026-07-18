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

#include "DALHAL_FileList.h"
#include "DALHAL_VirtualFile.h"

#include "Files/DALHAL_WebSocketAPI_index_html.h"
#include "Files/DALHAL_WebSocketAPI_main_css.h"
#include "Files/DALHAL_WebSocketAPI_main.js.h"
#include "Files/DALHAL_WebSocketAPI_log_helpers_js.h"

#include "Files/DALHAL_WebSocketAPI_wifi_config_css.h"

#include "Files/DALHAL_WebSocketAPI_autocomplete_js.h"
#include "Files/DALHAL_WebSocketAPI_wifi_config_js.h"

#include "Files/DALHAL_WebSocketAPI_modal_css.h"
#include "Files/DALHAL_WebSocketAPI_modal_js.h"

#include "Files/DALHAL_WebSocketAPI_control-panel_js.h"

namespace DALHAL {

    constexpr ApiVirtualFile VirtualFiles[] = {
        API_TEXT_FILE_ENTRY("index.html", WS_API_INDEX_HTML, MIME_HTML),
        API_TEXT_FILE_ENTRY("css/main.css", WS_API_MAIN_CSS, MIME_CSS),
        API_TEXT_FILE_ENTRY("css/modal.css", WS_API_MODAL_CSS, MIME_CSS),
        API_TEXT_FILE_ENTRY("css/wifi-config.css", WS_API_WIFI_CONFIG_CSS, MIME_CSS),

        API_TEXT_FILE_ENTRY("main.js", WS_API_MAIN_JS, MIME_JS),
        
        API_TEXT_FILE_ENTRY("modal.js", WS_API_MODAL_JS, MIME_JS),
        API_TEXT_FILE_ENTRY("control-panel.js", WS_API_CONTROL_PANEL_JS, MIME_JS),
        API_TEXT_FILE_ENTRY("log-helpers.js", WS_API_LOG_HELPERS_JS, MIME_JS),
        
        API_TEXT_FILE_ENTRY("wifi-config.js", WS_API_WIFI_CONFIG_JS, MIME_JS),
        API_TEXT_FILE_ENTRY("autocomplete.js", WS_API_AUTOCOMPLETE_JS, MIME_JS),
    };
    constexpr size_t VirtualFiles_Size = sizeof(VirtualFiles) / sizeof(VirtualFiles[0]);


    const ApiVirtualFile* GetApiVirtualFile(ZeroCopyString& zcFilePath) {

        for (size_t i = 0; i < VirtualFiles_Size; i++) {
            if (VirtualFiles[i].filename(&zcFilePath, nullptr)) {
                return &VirtualFiles[i];
            }
        }
        return nullptr;
    }

    void GetVirtualFiles(StringBuilderStreamer& sbs) {
        size_t totalSize = 0;
        for (size_t i = 0; i < VirtualFiles_Size; i++) {
            totalSize += VirtualFiles[i].size;
        }
        sbs.write_json_object_begin();
        sbs.write_jsonNumber(F("totalSize"), totalSize);
        sbs.write_json_value_separator();
        sbs.write_jsonMemberStart(F("files"));
        sbs.write_json_array_begin();
        for (size_t i = 0; i < VirtualFiles_Size; i++) {
            if (i>0) {
                sbs.write_json_value_separator();
            }
            sbs.write_json_object_begin();
            sbs.write_jsonMemberStart(F("name"));
            VirtualFiles[i].filename(nullptr, &sbs);
            sbs.write_json_value_separator();
            sbs.write_jsonNumber(F("size"), VirtualFiles[i].size);
            sbs.write_json_value_separator();
            sbs.write_jsonMemberStart(F("mime"));
            sbs.write_char('"');
            sbs.write_P(VirtualFiles[i].mime);
            sbs.write_char('"');

            sbs.write_json_object_end();
        }
        sbs.write_json_array_end();
        sbs.write_json_object_end();
    }
}

