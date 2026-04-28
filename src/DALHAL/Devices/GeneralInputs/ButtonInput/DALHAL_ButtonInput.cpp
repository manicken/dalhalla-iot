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

#include "DALHAL_ButtonInput.h"

#if defined(ESP8266) || defined(ESP32)
#include <DALHAL/API/DALHAL_WebSocketAPI.h> // for SendMessage
#else
#include <DALHAL_WebSocketAPI_Windows.h> // PC port - for SendMessage
#endif

#include "DALHAL_ButtonInput_JSON_Schema.h"

namespace DALHAL {

    __attribute__((used, externally_visible))
    constexpr Registry::DefineBase ButtonInput::RegistryDefine = {
        Create,
        &JsonSchema::ButtonInput::Root,
        DALHAL_REACTIVE_EVENT_TABLE(BUTTON_INPUT)
    };
    //volatile const void* keep_ButtonInput = &DALHAL::ButtonInput::RegistryDefine;

    // Factory method
    Device* ButtonInput::Create(DeviceCreateContext& context) {
        return new ButtonInput(context);
    }

    ButtonInput::~ButtonInput() {
        if (toggleTarget != nullptr) {
            delete toggleTarget;
            toggleTarget = nullptr;
        }
    }

    // Constructor
    ButtonInput::ButtonInput(DeviceCreateContext& context) : ButtonInput_DeviceBase(context.deviceType)
    {
        JsonSchema::ButtonInput::Extractors::Apply(context, this);
        pinMode(pin, INPUT);

        // Initial states
        stableState = digitalRead(pin);
        lastRaw = stableState;
        lastChangeMs = millis();
    }

    // Loop: call from main scheduler
    void ButtonInput::loop() {
        bool raw = digitalRead(pin);
        uint32_t now = millis();

        // Detect raw changes
        if (raw != lastRaw) {
            lastRaw = raw;
            lastChangeMs = now;
        }

        // If stable long enough and state changed
        if ((now - lastChangeMs) >= debounceMs && raw != stableState) {
            stableState = raw;

            bool pressed = activeLevel ? stableState : !stableState;

            if (pressed) {
#if HAS_REACTIVE_CUSTOM(BUTTON_INPUT)
                triggerPress();
#endif
                // Optional: call external device/action directly
                if (toggleTarget != nullptr) {
                    HALValue currValue;
                    HALOperationResult res = toggleTarget->ReadSimple(currValue);
                    if (res != HALOperationResult::Success) {
                        WebSocketAPI::Broadcast("[ButtonInput] pressed, toggleState could not execute: ", decodeUID(uid).c_str());
                        return;
                    } 
                    HALValue newVal = (currValue.asUInt() == 1) ? (uint32_t)0 : (uint32_t)1;
                    toggleTarget->WriteSimple(newVal);
                    WebSocketAPI::Broadcast("[ButtonInput] pressed, toggleState=" + newVal.asUInt());
                } else {
                    WebSocketAPI::Broadcast("[ButtonInput] pressed, toggleState could not execute because no targetdevice\r\n", decodeUID(uid).c_str());
                }

            } else {
#if HAS_REACTIVE_CUSTOM(BUTTON_INPUT)
                triggerRelease();
#endif
            }
#if HAS_REACTIVE_STATE_CHANGE(BUTTON_INPUT)
            triggerStateChange();
#endif
        }
    }

    // Read: returns the toggle state
    HALOperationResult ButtonInput::read(HALValue &val) {
        if (toggleTarget == nullptr) {
            return HALOperationResult::DeviceNotFound;
        }
        HALValue currValue;
        HALOperationResult res = toggleTarget->ReadSimple(currValue);
        if (res != HALOperationResult::Success) {
            Serial.printf("[ButtonInput] %s pressed, toggleState could not execute", decodeUID(uid).c_str());
            return res;
        } 
        val = currValue;
        return HALOperationResult::Success;
    }

    // ToString for JSON dump/debug
    String ButtonInput::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";

        ret += DeviceConstStrings::type;
        ret += this->Type;
        ret += ",";

        ret += DeviceConstStrings::pin;
        ret += std::to_string(pin).c_str();


        return ret;
    }

} // namespace DALHAL
