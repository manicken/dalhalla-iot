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

#include <PubSubClient.h>
#include <string>

#include <DALHAL/Core/Types/DALHAL_Device.h>
#include "DALHAL_HA_Device.h"

namespace DALHAL {

    class HA_DeviceEntity : public HA_Device {
    public:

        HA_DeviceEntity(HA_CreateFunctionContext& context);
        
        PubSubClient& mqttClient;
        std::string hass_uid;

        virtual HALOperationResult ha_apply(const ZeroCopyString& zcVal);

        HA_DeviceEntity* findHassDevice(const ZeroCopyString& zcHassUid) override;
    };

}