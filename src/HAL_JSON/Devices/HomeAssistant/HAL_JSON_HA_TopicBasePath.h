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

#pragma once

#include "../../HAL_JSON_ZeroCopyString.h"

namespace HAL_JSON {

    enum class TopicBasePathMode {
        BaseRoot,
        State,
        Status,
        Command
    };
    
    struct TopicSuffix {
        TopicBasePathMode mode;
        const char* str;
        int strLength;
    };

    const TopicSuffix& GetTopicSuffix(TopicBasePathMode mode);

    

    class TopicBasePath {
    private:
        /** this is the actual string */
        char* pathStr;
        /** just points to somewhere in pathStr */
        char* pathStrDynamicStart;

        TopicBasePath(TopicBasePath&) = delete;
        TopicBasePath(TopicBasePath&&) = delete;
    public:
        TopicBasePath();
        ~TopicBasePath();
        const char* SetAndGet(TopicBasePathMode mode);
        ZeroCopyString GetDeviceId();
        void Set(const char* deviceIdStr, const char* uidStr);
    };

}