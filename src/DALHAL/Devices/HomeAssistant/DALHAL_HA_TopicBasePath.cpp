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

#include "DALHAL_HA_TopicBasePath.h"
#include "DALHAL_HA_Constants.h"
#include <stdio.h> // snprintf
#include <string.h> // strlen

namespace DALHAL {

    
    static const TopicSuffix suffixTable[] = {
        { TopicBasePathMode::BaseRoot,     "", 0},
        { TopicBasePathMode::State,       DALHAL_HOME_ASSISTANT_TOPICBASEPATH_STATE, sizeof(DALHAL_HOME_ASSISTANT_TOPICBASEPATH_STATE)-1 },
        { TopicBasePathMode::Status,      DALHAL_HOME_ASSISTANT_TOPICBASEPATH_STATUS, sizeof(DALHAL_HOME_ASSISTANT_TOPICBASEPATH_STATUS)-1 },
        { TopicBasePathMode::Command,     DALHAL_HOME_ASSISTANT_TOPICBASEPATH_COMMAND, sizeof(DALHAL_HOME_ASSISTANT_TOPICBASEPATH_COMMAND)-1}
        
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

        // Compute <DALHAL_DEVICES_HOME_ASSISTANT_ROOTNAME>/<device>/<uid>/ length
        int baseLen = snprintf(nullptr, 0,
                            DALHAL_DEVICES_HOME_ASSISTANT_ROOTNAME "/%s/%s/", deviceIdStr, uidStr);

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
                DALHAL_DEVICES_HOME_ASSISTANT_ROOTNAME "/%s/%s/", deviceIdStr, uidStr);

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