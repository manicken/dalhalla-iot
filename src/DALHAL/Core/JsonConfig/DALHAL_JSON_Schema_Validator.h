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

#include "DALHAL_JSON_Schema.h"

#include <ArduinoJSON.h>
using json = JsonVariant;

namespace DALHAL {

    namespace JsonSchema {

        struct ValidateFromRegisterContext {

            enum class State {
                Enabled,
                Disabled
            };
            bool* validDevicesArray = nullptr;
            int validDevicesArraySize = 0;
            int validDevicesCount = 0;
            State state = State::Disabled;

            inline void Init(int validDevicesArraySize) {
                if (state == State::Disabled) return;

                delete[] validDevicesArray;
                this->validDevicesArraySize = validDevicesArraySize;
                validDevicesArray = new bool[validDevicesArraySize]();
                validDevicesCount = 0;
            }
            ValidateFromRegisterContext(State state): state(state) { }
            ValidateFromRegisterContext() = delete;
            ValidateFromRegisterContext(const ValidateFromRegisterContext&) = delete;

            // move functionality
            ValidateFromRegisterContext(ValidateFromRegisterContext&& other) noexcept {
                validDevicesArray = other.validDevicesArray;
                validDevicesArraySize = other.validDevicesArraySize;
                validDevicesCount = other.validDevicesCount;
                state = other.state;

                other.validDevicesArray = nullptr;
                other.validDevicesArraySize = 0;
                other.validDevicesCount = 0;
            }

            ~ValidateFromRegisterContext() {
                delete[] validDevicesArray;
            }

            void SetDevice(int index, bool value) {
                if ((state == State::Enabled) && validDevicesArray && (index >= 0) && (index < validDevicesArraySize)) {
                    
                    if (value && !validDevicesArray[index]) {
                        ++validDevicesCount;
                    }
                    validDevicesArray[index] = value;
                }
            }
        };

        bool isKnownField(const char* key, const FieldBase* const* fields);
        // Helper to validate FieldString / FieldUID
        void validateStringField(const JsonVariant& value, const FieldString* f, bool& anyError);
        // Validate a single field
        void validateField(const JsonVariant& j, const FieldBase* field, bool& anyError);
        // Validate AnyOfGroup
        void validateAnyOfGroup(const JsonVariant& j, const AnyOfGroup* group, bool& anyError);
        // Validate AllOfGroup
        void validateAllOfGroup(const JsonVariant& j, const AllOfGroup* group, bool& anyError);
        // Validate ModeSelector
        int evaluateModes(const JsonVariant& j, const ModeSelector* modes);
        // Validate a full device
        /*int*/ void validateJsonObject(const JsonVariant& j, const JsonSchema::JsonObjectScheme* jsonObjectScheme, bool& anyError);
        // Validate the JSON array against the given device registry.
        void validateFromRegister(const JsonVariant& jsonArray, const Registry::Item* reg, ValidateFromRegisterContext& context, bool& anyError);

    }

}