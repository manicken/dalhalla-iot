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

#include <DALHAL/Core/Device/DALHAL_CachedDeviceRead.h>
#include <DALHAL/Core/Types/DALHAL_ZeroCopyString.h>
#include <DALHAL/Core/Reactive/DALHAL_ReactiveEvent.h>

namespace DALHAL {

    struct ThingSpeakField {
        int index;
        ReactiveEvent* reactiveEvent;
        CachedDeviceRead* cdr;
        bool dataReady = false;
        bool sendAllInSync = false;

        ThingSpeakField();
        ~ThingSpeakField();

        void Set(int index, const char* uidPathAndFuncName_cStr, bool _sendAllInSync = false);

        inline bool DataReady() {
            return true;
            if (sendAllInSync) {
                if (dataReady) return true;
                else if (reactiveEvent == nullptr) { dataReady = true; }
                else if (reactiveEvent->CheckForEvent()) {  dataReady = true; }
                return false;
            } else {
                if (reactiveEvent == nullptr) return true; // allways ready 
                return reactiveEvent->CheckForEvent();
                return false;
            }
        }
        inline void ClearDataReady() {
            dataReady = false;
        }
    };
}