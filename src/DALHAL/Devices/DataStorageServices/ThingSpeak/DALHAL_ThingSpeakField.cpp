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

#include "DALHAL_ThingSpeakField.h"

#include <DALHAL/Core/Manager/DALHAL_DeviceManager.h> // DeviceManager::GetDeviceEvent
#include <DALHAL/Support/DALHAL_Logger.h>

namespace DALHAL {
    
    ThingSpeakField::ThingSpeakField() : index(0), reactiveEvent(nullptr), cdr(nullptr) {}

    ThingSpeakField::~ThingSpeakField() {
        if (reactiveEvent != nullptr) {
            delete reactiveEvent;
        }
    }

    void ThingSpeakField::Set(int fieldIndex, const char* uidPathAndFuncName_cStr, bool _sendAllInSync) {
        this->sendAllInSync = _sendAllInSync;
        this->index = fieldIndex;
        cdr = new CachedDeviceRead();
        ZeroCopyString zcStrUidPathAndFuncName(uidPathAndFuncName_cStr);
        if (cdr->Set(zcStrUidPathAndFuncName)) {
            HALOperationResult res = DeviceManager::GetDeviceEvent(zcStrUidPathAndFuncName, &reactiveEvent);
            if (res != HALOperationResult::Success) {
                reactiveEvent = nullptr;
                GlobalLogger.Error(F("error while GetDeviceEvent:"), HALOperationResultToString(res));
            }
        }
        
    }

}