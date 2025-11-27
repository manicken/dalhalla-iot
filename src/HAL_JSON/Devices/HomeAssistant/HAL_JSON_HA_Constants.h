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

#define HAL_JSON_DEVICES_HOME_ASSISTANT_ROOTNAME "dalhal"
#define HAL_JSON_HOME_ASSISTANT_TOPICBASEPATH_STATUS "status"
#define HAL_JSON_HOME_ASSISTANT_TOPICBASEPATH_STATE "state"
#define HAL_JSON_HOME_ASSISTANT_TOPICBASEPATH_COMMAND "command"
#define HAL_JSON_HOME_ASSISTANT_AVAILABILITY_ONLINE "online"
#define HAL_JSON_HOME_ASSISTANT_AVAILABILITY_OFFLINE "offline"


#define JSON(...) #__VA_ARGS__

namespace HAL_JSON {

}