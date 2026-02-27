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

#define DALHAL_VALIDATE_JSON_STRICT

#if defined(DALHAL_VALIDATE_JSON_STRICT)
#define DALHAL_VALIDATE_IN_LOOP_FAIL_OPERATION return false
#define DALHAL_VALIDATE_IN_LOOP_FAIL_OPERATION_NO_RET_VAL return
#else
#define DALHAL_VALIDATE_IN_LOOP_FAIL_OPERATION continue
#define DALHAL_VALIDATE_IN_LOOP_FAIL_OPERATION_NO_RET_VAL continue
#endif

#define DALHAL_USE_EFFICIENT_FIND

#define DALHAL_ERROR_SOURCE_ ""
#define DALHAL_ERROR_SOURCE_MGR_VERIFY_DEVICE "MGR_VDJ"
#define DALHAL_ERROR_SOURCE_DHT_VERIFY_JSON "DHT_VJ"
#define DALHAL_ERROR_SOURCE_1WTD_VERIFY_JSON "1WTD_VJ"
#define DALHAL_ERROR_SOURCE_REGO600_REG_VERIFY_JSON "REGO600reg_VJ"
#define DALHAL_ERROR_SOURCE_TX433_VERIFY_JSON "TX433_VJ"
#define DALHAL_ERROR_SOURCE_TX433_UNIT_VERIFY_JSON "TX433unit_VJ"
#define DALHAL_ERROR_SOURCE_TX433_UNIT_VERIFY_FC_JSON "TX433unit_VFCJ"
#define DALHAL_ERROR_SOURCE_I2C_VERIFY_JSON "I2C_VJ"
#define DALHAL_ERROR_SOURCE_DISPLAY_SSD1306_VERIFY_JSON "SSD1306_VJ"
#define DALHAL_ERROR_SOURCE_DISPLAY_SSD1306_ELM_VERIFY_JSON "SSD1306_ELM_VJ"
#define DALHAL_ERROR_SOURCE_THINGSPEAK_VERIFY_JSON "ThiSpk_VJ"
#define DALHAL_ERROR_SOURCE_WS2812_VERIFY_JSON "WS2812_VJ"
#define DALHAL_ERROR_SOURCE_REMOTE_FETCH_REST_VERIFY_JSON "REM_REST_VJ"

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

