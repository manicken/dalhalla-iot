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

#include "DALHAL_SimpleEventDevice.h"

namespace DALHAL
{
    SimpleEventDevice::Context::Context(uint32_t& _current) : current(_current) { }

    bool SimpleEventDevice::EventCheck(void* context) {
        SimpleEventDevice::Context* ctx = static_cast<SimpleEventDevice::Context*>(context);
        if (ctx->current != ctx->lastSeen) {
            ctx->lastSeen = ctx->current; // update to current
            return true;
        }
        return false;
    }

    HALOperationResult SimpleEventDevice::Get_DeviceEvent(ZeroCopyString& zcStrFuncName, DeviceEvent** deviceEventOut)
    {
        if (deviceEventOut != nullptr) {
#if (DALHAL_REACTIVE_CFG_SCRIPT_VARIABLE & DALHAL_REACTIVE_FEATURE_VALUE_CHANGE)
            if (zcStrFuncName.EqualsIC("value_change")) {
                *deviceEventOut = new DeviceEvent(
                                    SimpleEventDevice::EventCheck, 
                                    &DeviceEvent::DeleteAs<SimpleEventDevice::Context>, 
                                    new SimpleEventDevice::Context(valueChangeCounter));
                return HALOperationResult::Success; 
            }
#endif
            /*if (zcStrFuncName.EqualsIC("state_change")) {
                *deviceEventOut = new DeviceEvent(
                                        SimpleEventDevice::EventCheck, 
                                        &DeviceEvent::DeleteAs<SimpleEventDevice::Context>, 
                                        new SimpleEventDevice::Context(stateChangeCounter));
                return HALOperationResult::Success; 
            }
            if (zcStrFuncName.EqualsIC("write")) {
                *deviceEventOut = new DeviceEvent(
                                        SimpleEventDevice::EventCheck, 
                                        &DeviceEvent::DeleteAs<SimpleEventDevice::Context>, 
                                        new SimpleEventDevice::Context(writeCounter));
                return HALOperationResult::Success; 
            }
            if (zcStrFuncName.EqualsIC("read")) {
                *deviceEventOut = new DeviceEvent(
                                        SimpleEventDevice::EventCheck, 
                                        &DeviceEvent::DeleteAs<SimpleEventDevice::Context>, 
                                        new SimpleEventDevice::Context(readCounter));
                return HALOperationResult::Success; 
            }
            if (zcStrFuncName.EqualsIC("exec")) {
                *deviceEventOut = new DeviceEvent(
                                        SimpleEventDevice::EventCheck, 
                                        &DeviceEvent::DeleteAs<SimpleEventDevice::Context>, 
                                        new SimpleEventDevice::Context(execCounter));
                return HALOperationResult::Success; 
            }*/
        }
        // return Success even if deviceEventOut == nullptr 
        // this is used to test if a device support events without allocating a DeviceEvent instance
        return HALOperationResult::Success; 
    }

} // namespace DALHAL