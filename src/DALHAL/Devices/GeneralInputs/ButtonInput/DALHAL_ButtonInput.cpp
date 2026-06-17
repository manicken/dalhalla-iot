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

#include <DALHAL/API/DALHAL_BlockStreamer.h>

#include "DALHAL_ButtonInput_JSON_Schema.h"

namespace DALHAL {

    __attribute__((used, externally_visible))
    constexpr Registry::DefineBase ButtonInput::RegistryDefine = {
        Create,
        &JsonSchema::ButtonInput::Root,
        DALHAL_REACTIVE_EVENT_TABLE(BUTTON_INPUT),
        &ButtonInput::FunctionTable
    };

    /* override */
    const Registry::DefineBase* ButtonInput::GetRegistryDefine() {
        return &RegistryDefine;
    }

    constexpr FunctionEntry<FunctionTypes::ReadToHALValue> ButtonInput::readValueFunctions[] = {
        DALHAL_PRIMARY_FUNCTION_ENTRY(HALValue_primary_read, "read  current state")
    };

    __attribute__((used, externally_visible))
    constexpr DeviceFunctionTable ButtonInput::FunctionTable = {
        EmptyFunctionTable<FunctionTypes::Exec>,
        DALHAL_FUNCTION_TABLE_ENTRY(readValueFunctions),
        EmptyFunctionTable<FunctionTypes::WriteHALValue>,
        EmptyFunctionTable<FunctionTypes::BracketOpRead>,
        EmptyFunctionTable<FunctionTypes::BracketOpWrite>,
        EmptyFunctionTable<FunctionTypes::ReadString>,
        EmptyFunctionTable<FunctionTypes::WriteString>,
    };

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

                        DALHAL::BlockStreamer bs(DALHAL::WebSocketAPI::BroadcastCb, "log entry", DALHAL::BlockStreamer::DataType::PlainText);
                        bs.writer().write(F("[ButtonInput] pressed, toggleState could not execute: "));
                        decodeUID(uid, bs.writer());
                        
                        return;
                    } 
                    HALValue newVal = currValue.toBool() ? false : true;
                    toggleTarget->WriteSimple(newVal);

                    DALHAL::BlockStreamer bs(DALHAL::WebSocketAPI::BroadcastCb, "log entry", DALHAL::BlockStreamer::DataType::PlainText);
                    bs.writer().write(F("[ButtonInput] pressed, toggleState="));
                    newVal.toString(bs.writer());
                
                } else {

                    DALHAL::BlockStreamer bs(DALHAL::WebSocketAPI::BroadcastCb, "log entry", DALHAL::BlockStreamer::DataType::PlainText);
                    bs.writer().write(F("[ButtonInput] pressed, toggleState could not execute because no targetdevice"));
                    decodeUID(uid, bs.writer());
                    
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
    HALOperationResult ButtonInput::HALValue_primary_read(Device* device, HALValue& val) {
        ButtonInput& self = static_cast<ButtonInput&>(*device);
        val = self.activeLevel ?
              self.stableState :
              !self.stableState;
        return HALOperationResult::Success;
    }

    void ButtonInput::PrintTo(StringBuilderStreamer& sbs) {
        Device::PrintTo(sbs);

        sbs.write_json_value_separator();
        sbs.write_jsonNumber(F("pin"), pin);
        sbs.write_json_value_separator();
        sbs.write_jsonNumber(F("activeLevel"), activeLevel);
        sbs.write_json_value_separator();
        sbs.write_jsonNumber(F("debounceMs"), debounceMs);

    }

} // namespace DALHAL
