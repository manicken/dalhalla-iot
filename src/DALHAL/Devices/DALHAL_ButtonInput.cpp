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

#include "DALHAL_ButtonInput.h"
#include "../Core/Device/DALHAL_JSON_Config_Defines.h"
#include "../Support/DALHAL_ArduinoJSON_ext.h"
#include "../API/DALHAL_WebSocketAPI.h"
#include "../Core/Manager/DALHAL_GPIO_Manager.h"

namespace DALHAL {

// Factory method
Device* ButtonInput::Create(const JsonVariant &jsonObj, const char* type) {
    return new ButtonInput(jsonObj, type);
}

// JSON validation
bool ButtonInput::VerifyJSON(const JsonVariant &jsonObj) {
    
    return GPIO_manager::ValidateJsonAndCheckIfPinAvailableAndReserve(
        jsonObj, static_cast<uint8_t>(GPIO_manager::PinFunc::IN));
}

ButtonInput::~ButtonInput() {
    if (toggleTarget != nullptr) {
        delete toggleTarget;
        toggleTarget = nullptr;
    }
}

// Constructor
ButtonInput::ButtonInput(const JsonVariant &jsonObj, const char* type)
    : Device(type)
{
    pin = GetAsUINT32(jsonObj, DALHAL_KEYNAME_PIN);
    uid = encodeUID(GetAsConstChar(jsonObj, DALHAL_KEYNAME_UID));

    // Optional debounce, default 30ms
    debounceMs = jsonObj.containsKey("debounce_ms") ? jsonObj["debounce_ms"].as<uint32_t>() : 30;

    // Active level (default HIGH)
    activeLow = jsonObj.containsKey("active") && strcmp(jsonObj["active"], "low") == 0;

    GPIO_manager::ReservePin(pin);
    pinMode(pin, activeLow ? INPUT_PULLUP : INPUT);

    // Initial states
    stableState = digitalRead(pin);
    lastRaw = stableState;
    lastChangeMs = millis();

    // Optional external action target
    if (jsonObj.containsKey("on_press")) {
        toggleTarget = new CachedDeviceAccess();
        if (toggleTarget->Set(jsonObj["on_press"].as<const char*>()) == false) {
            delete toggleTarget;
            toggleTarget = nullptr;
        }
    }
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

    // Debounce and detect stable change
    if ((now - lastChangeMs) >= debounceMs && raw != stableState) {
        stableState = raw;

        bool pressed = activeLow ? !stableState : stableState;

        if (pressed) {
            // TEMP toggle

            //toggleState = !toggleState;
            
            HALValue newVal;
            // Optional: call external device/action
            if (toggleTarget != nullptr) {
                HALValue currValue;
                HALOperationResult res = toggleTarget->ReadSimple(currValue);
                if (res != HALOperationResult::Success) {
                    Serial.printf("[ButtonInput] %s pressed, toggleState could not execute\r\n", decodeUID(uid).c_str());
                    WebSocketAPI::SendMessage("[ButtonInput] pressed, toggleState could not execute");
                    return;
                } 
                newVal = (currValue.asUInt() == 1) ? (uint32_t)0 : (uint32_t)1;
                toggleTarget->WriteSimple(newVal);
                Serial.printf("[ButtonInput] %s pressed, toggleState=%d\r\n", decodeUID(uid).c_str(), newVal.asUInt());
                WebSocketAPI::SendMessage("[ButtonInput] pressed, toggleState=" + newVal.asUInt());
            } else {
                Serial.printf("[ButtonInput] %s pressed, toggleState could not execute because no targetdevice\r\n", decodeUID(uid).c_str());
                WebSocketAPI::SendMessage("[ButtonInput] pressed, toggleState could not execute because no targetdevice\r\n");
            }

        }
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
    ret += type;
    ret += ",";

    ret += DeviceConstStrings::pin;
    ret += std::to_string(pin).c_str();


    return ret;
}

} // namespace DALHAL
