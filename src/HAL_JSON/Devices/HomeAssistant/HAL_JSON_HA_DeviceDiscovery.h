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

#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

namespace HAL_JSON
{
    class HA_DeviceDiscovery {
    public:
        static void StartSendBaseData(const JsonVariant &jsonObj, PubSubClient& mqtt, int packetLength);
        static void SendBaseData(const JsonVariant &jsonObj, const JsonVariant& jsonObjDeviceGroup, const char* rootName, PubSubClient& mqtt);
    };

    class PSC_JsonWriter {
    public:
        static void key(PubSubClient& mqtt, const char* key);
        static void kv(PubSubClient& mqtt, const char* key, const char* value, bool last = false);

    };
} // namespace HAL_JSON
