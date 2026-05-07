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

#pragma once

#define DALHAL_DEV_HOME_ASSISTANT_DD_CONFIG_TOPIC_HEAD    "homeassistant"
#define DALHAL_DEV_HOME_ASSISTANT_DD_CONFIG_TOPIC_TAIL    "config"

#define DALHAL_DEV_HOME_ASSISTANT_DD_BASENAME             "dalhal"
#define DALHAL_DEV_HOME_ASSISTANT_DD_STATUS_TOPIC_TAIL    "status"
#define DALHAL_DEV_HOME_ASSISTANT_DD_STATE_TOPIC_TAIL     "state"
#define DALHAL_DEV_HOME_ASSISTANT_DD_COMMAND_TOPIC_TAIL   "command"
#define DALHAL_DEV_HOME_ASSISTANT_DD_AVAILABILITY_ONLINE  "online"
#define DALHAL_DEV_HOME_ASSISTANT_DD_AVAILABILITY_OFFLINE "offline"

#define DALHAL_DEV_HOME_ASSISTANT_DD_CONFIG_TOPIC_FMT                    DALHAL_DEV_HOME_ASSISTANT_DD_CONFIG_TOPIC_HEAD "/%s/" DALHAL_DEV_HOME_ASSISTANT_DD_BASENAME "_%012llX/%s_%s/" DALHAL_DEV_HOME_ASSISTANT_DD_CONFIG_TOPIC_TAIL
#define DALHAL_DEV_HOME_ASSISTANT_DD_CONFIG_CLEANUP_SUBSCRIBE_TOPIC_FMT  DALHAL_DEV_HOME_ASSISTANT_DD_CONFIG_TOPIC_HEAD "/+/" DALHAL_DEV_HOME_ASSISTANT_DD_BASENAME "_%012llX/+/" DALHAL_DEV_HOME_ASSISTANT_DD_CONFIG_TOPIC_TAIL

#define DALHAL_DEV_HOME_ASSISTANT_DD_COMMAND_SUBSCRIBE_TOPIC_FMT         DALHAL_DEV_HOME_ASSISTANT_DD_BASENAME "/%s/+/" DALHAL_DEV_HOME_ASSISTANT_DD_COMMAND_TOPIC_TAIL

#define JSON(...) #__VA_ARGS__

namespace DALHAL {

}