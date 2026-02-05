#include "HAL_JSON_ButtonInput.h"


namespace HAL_JSON {

// Factory method
Device* ButtonInput::Create(const JsonVariant &jsonObj, const char* type) {
    return new ButtonInput(jsonObj, type);
}

// JSON validation
bool ButtonInput::VerifyJSON(const JsonVariant &jsonObj) {
    
    return GPIO_manager::ValidateJsonAndCheckIfPinAvailableAndReserve(
        jsonObj, static_cast<uint8_t>(GPIO_manager::PinFunc::IN));
}

// Constructor
ButtonInput::ButtonInput(const JsonVariant &jsonObj, const char* type)
    : Device(type)
{
    pin = GetAsUINT32(jsonObj, HAL_JSON_KEYNAME_PIN);
    uid = encodeUID(GetAsConstChar(jsonObj, HAL_JSON_KEYNAME_UID));

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
    toggleState = false;

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
            toggleState = !toggleState;

            // Optional: call external device/action
            if (toggleTarget != nullptr) {
                HALValue val = toggleState ? (uint32_t)1 : (uint32_t)0;
                toggleTarget->WriteSimple(val);
            }

            // Debug
            Serial.printf("[ButtonInput] %s pressed, toggleState=%d\n", decodeUID(uid).c_str(), toggleState);
        }
    }
}

// Read: returns the toggle state
HALOperationResult ButtonInput::read(HALValue &val) {
    val = (uint32_t)toggleState;
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
    ret += ",";

    ret += DeviceConstStrings::value;
    ret += std::to_string(toggleState).c_str();
    return ret;
}

} // namespace HAL_JSON
