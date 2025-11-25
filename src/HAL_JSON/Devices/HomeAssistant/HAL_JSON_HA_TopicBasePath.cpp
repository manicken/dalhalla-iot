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

#include "HAL_JSON_HA_TopicBasePath.h"
#include <stdio.h> // snprintf
#include <string.h> // strlen

namespace HAL_JSON {

    
    static const TopicSuffix suffixTable[] = {
        { TopicBasePathMode::BaseRoot,     "", 0},
        { TopicBasePathMode::State,       "state", sizeof("state")-1 },
        { TopicBasePathMode::Status,      "status", sizeof("status")-1 },
        { TopicBasePathMode::Availability,"active", sizeof("active")-1 },
        { TopicBasePathMode::Command,     "command", sizeof("command")-1}
        
    };

    const TopicSuffix& GetTopicSuffix(TopicBasePathMode mode) {
        for (auto& s : suffixTable) {
            if (s.mode == mode) {
                return s;
            }
        }
        return suffixTable[0];
    }

    TopicBasePath::TopicBasePath() : pathStr(nullptr), pathStrDynamicStart(nullptr) {}

    void TopicBasePath::Set(const char* deviceIdStr, const char* uidStr) {
        if (pathStr != nullptr) return; // can/should only be set once

        // Compute dalhal/<device>/<uid>/ length
        int baseLen = snprintf(nullptr, 0,
                            "dalhal/%s/%s/", deviceIdStr, uidStr);

        // Compute longest suffix
        size_t maxSuffixLen = 0;
        for (auto& s : suffixTable) {
            size_t len = s.strLength;
            if (len > maxSuffixLen) maxSuffixLen = len;
        }

        size_t allocatedSize = baseLen + maxSuffixLen + 1;
        pathStr = new char[allocatedSize]();

        // Write the base only
        snprintf(pathStr, allocatedSize,
                "dalhal/%s/%s/", deviceIdStr, uidStr);

        // Dynamic part begins here
        pathStrDynamicStart = pathStr + baseLen;

    }

    ZeroCopyString TopicBasePath::GetDeviceId() {
        ZeroCopyString tmp(pathStr);
        tmp.start = tmp.FindChar('/')+1;
        tmp.end = tmp.FindChar('/');
        return tmp;
    }

    const char* TopicBasePath::SetAndGet(TopicBasePathMode mode) {
        if (pathStr == nullptr || pathStrDynamicStart == nullptr) return pathStr;

        const TopicSuffix& suffix = GetTopicSuffix(mode);

        if (suffix.mode == TopicBasePathMode::BaseRoot) {
            *pathStrDynamicStart = '\0'; // null terminate here
        } else if (pathStrDynamicStart != nullptr) {
            size_t len = suffix.strLength;
            memcpy(pathStrDynamicStart, suffix.str, len);
            pathStrDynamicStart[len] = '\0';
        }
        return pathStr;
    }

    TopicBasePath::~TopicBasePath() {
        pathStrDynamicStart = nullptr; // non owned just a ptr to somewhere in pathStr
        delete[] pathStr;
        pathStr = nullptr;

    }
}