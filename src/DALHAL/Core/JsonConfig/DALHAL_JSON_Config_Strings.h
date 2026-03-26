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

// Global use
#define DALHAL_KEYNAME_DISABLED                           "disabled"
#define DALHAL_KEYNAME_TYPE                               "type"
#define DALHAL_KEYNAME_UID                                "uid"
#define DALHAL_KEYNAME_PIN                                "pin"
#define DALHAL_KEYNAME_RXPIN                              "rxpin"
#define DALHAL_KEYNAME_TXPIN                              "txpin"
#define DALHAL_KEYNAME_NOTE                               "note"
#define DALHAL_KEYNAME_ITEMS                              "items"
#define DALHAL_KEYNAME_REFRESHTIME_SEC                    "refreshtimesec"
#define DALHAL_KEYNAME_REFRESHTIME_MIN                    "refreshtimemin"

// Single Pulse Output
#define DALHAL_KEYNAME_SINGLE_PULSE_OUTPUT_INACTIVE_STATE       "istate"
#define DALHAL_KEYNAME_SINGLE_PULSE_OUTPUT_DEFAULT_PULSE_LENGHT "plength"

// PWM output (also analog write)
#define DALHAL_KEYNAME_PWM_CFG_FREQUENCY                  "freq"
#define DALHAL_KEYNAME_PWM_CFG_RESOLUTION                 "res"
#define DALHAL_KEYNAME_PWM_INVOUT                         "invOut"


// the following must be at end
#define DALHAL_ERR_MISSING_STRING_VALUE_KEY (F("Missing string value key: "))
#define DALHAL_ERR_VALUE_TYPE_NOT_STRING (F("Value type not string: "))
#define DALHAL_ERR_STRING_EMPTY (F("String is empty:"))

#define DALHAL_ERR_MISSING_KEY(k) (F("Missing key: " k))
#define DALHAL_ERR_VALUE_TYPE(t) (F("Value type: " t))
#define DALHAL_ERR_ITEMS_EMPTY(t) (F(t " items list is empty"))
#define DALHAL_ERR_ITEMS_NOT_VALID(t) (F(t " do not contain any valid items"))

#define DALHAL_DEBUG(constStr, dynStr) Serial.print(constStr); Serial.println(dynStr);
//#define DALHAL_DEBUG(constStr, dynStr) GlobalLogger.Info(constStr, dynStr);

