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

#include "DALHAL_JSON_Schema_Validator.h"
#include "DALHAL/Core/Manager/DALHAL_GPIO_Manager.h"
#include <DALHAL/Support/DALHAL_Logger.h>
#include <DALHAL/Core/Types/DALHAL_ZeroCopyString.h>

#include <ArduinoJSON.h>
using json = JsonVariant;
 
namespace DALHAL {

    namespace JsonSchema {

        // this is only a helper/support function and do not use anyError
        // as this could be jsut defined as a warning, depending on strict level requirements
        bool isKnownField(const char* key, const FieldBase* const* fields)
        {
            for (int i = 0; fields[i] != nullptr; i++) {
                const FieldBase* f = fields[i];

                if (f->type == FieldType::AnyOfGroup) {
                    const AnyOfGroup* group = static_cast<const AnyOfGroup*>(f);

                    for (int g = 0; group->fields[g] != nullptr; g++) {
                        if (strcmp(key, group->fields[g]->name) == 0)
                            return true;
                    }
                }
                else {
                    if (strcmp(key, f->name) == 0)
                        return true;
                }
            }

            return false;
        }

        // Helper to validate FieldString / FieldUID
        void validateStringField(const JsonVariant& value, const FieldString* f, bool& anyError)
        {
            if (!value.is<const char*>()) {
                GlobalLogger.Error(F("Field must be a string:"), f->name);
                anyError = true;
                return;
            }
            ZeroCopyString zcStr = value.as<const char*>(); // wrap in ZeroCopyString for neat functions
            zcStr.Trim();
            size_t strLen = zcStr.Length(); // use of lenght here is fast
            if (strLen == 0) {
                GlobalLogger.Error(F("Field string cannot be empty:"), f->name);
            }
            if (f->maxLength > 0 && strLen > f->maxLength) {
                GlobalLogger.Error(F("Field exceeds maxLength:"), f->name);
                anyError = true;
                return;
            }
        }

        // Validate a single field
        void validateField(const JsonVariant& j, const FieldBase* field, bool& anyError)
        {
            if (!field) { return; } // failsafe just return

            // Handle required/optional
            if (!j.containsKey(field->name)) {
                if (field->flag == FieldFlag::Required) {
                    GlobalLogger.Error(F("Required field missing"));
                    GlobalLogger.setLastEntrySource(field->name); // use this to avoid copy of flash string
                    anyError = true;
                    return;
                }
                return; // optional can be missing
            }

            JsonVariant value = j[field->name];

            switch (field->type) {
                case FieldType::Int: {
                    auto f = static_cast<const FieldInt*>(field);
                    if (!value.is<int>()) {
                        GlobalLogger.Error(F(" must be a integer"));
                        GlobalLogger.setLastEntrySource(field->name); // use this to avoid copy of flash string
                        anyError = true;
                        return;
                    }
                    int v = value.as<int>();
                    if (v < f->minValue || v > f->maxValue) {
                        GlobalLogger.Error(F(" out of range"));
                        GlobalLogger.setLastEntrySource(field->name); // use this to avoid copy of flash string
                        anyError = true;
                        return;
                    }
                    break;
                }
                case FieldType::UInt: {
                    auto f = static_cast<const FieldUInt*>(field);
                    if (!value.is<unsigned int>()) {
                        GlobalLogger.Error(F(" must be unsigned int"));
                        GlobalLogger.setLastEntrySource(field->name); // use this to avoid copy of flash string
                        anyError = true;
                        return;
                    }
                    unsigned int v = value.as<unsigned int>();
                    if (v < f->minValue || v > f->maxValue) {
                        GlobalLogger.Error(F(" out of range"));
                        GlobalLogger.setLastEntrySource(field->name); // use this to avoid copy of flash string
                        anyError = true;
                        return;
                    }
                    break;
                }
                case FieldType::Float: {
                    auto f = static_cast<const FieldFloat*>(field);
                    if (!value.is<float>() && !value.is<double>() && !value.is<int>()) {
                        GlobalLogger.Error(F(" must be a number"));
                        GlobalLogger.setLastEntrySource(field->name); // use this to avoid copy of flash string
                        anyError = true;
                        return;
                    }
                    float v = value.as<float>();
                    if (v < f->minValue || v > f->maxValue) {
                        GlobalLogger.Error(F(" out of range"));
                        GlobalLogger.setLastEntrySource(field->name); // use this to avoid copy of flash string
                        anyError = true;
                        return;
                    }
                    break;
                }
                case FieldType::HardwarePin: {
                    auto f = static_cast<const FieldHardwarePin*>(field);
                    if (value.is<uint8_t>() == false) {
                        GlobalLogger.Error(F("Harware Pin is not a valid value"));
                        GlobalLogger.setLastEntrySource(f->name); //  safe as field name is a flash const and setLastEntrySource require "static" strings as it just store the ptr to the string
                    }
                    uint8_t pin = value.as<uint8_t>();
                    if (GPIO_manager::CheckIfPinAvailableAndReserve(pin, f->mode) == false)
                        anyError = true;
                    return;
                }
                case FieldType::HardwarePinOrVirtualPin: {
                    auto f = static_cast<const FieldHardwarePinOrVirtualPIN*>(field);
                    if (value.is<const char*>()) {
                        bool anyErrorTemp = false;
                        validateStringField(value, static_cast<const FieldString*>(field), anyErrorTemp);
                        if (anyErrorTemp == true) {
                            anyError = true;
                            return;
                        }
                        // TODO implement functionality to check if "virtual pin device" exists
                    } else {

                    }
                    return;
                }
                case FieldType::UID:
                case FieldType::UID_Path:
                    // cast FieldString for UID / UID_Path / simple string fields
                    validateStringField(value, static_cast<const FieldString*>(field), anyError);
                    return;
                case FieldType::Array: {
                    auto f = static_cast<const FieldArray*>(field);
                    validateDevice(value, f->subtype, anyError);
                    return;
                }
                case FieldType::Object: {
                    auto f = static_cast<const FieldObject*>(field);
                    validateDevice(value, f->subtype, anyError);
                    return;
                }
                case FieldType::AnyOfGroup:
                    // handled by validateAnyOfGroup
                    break;
            }
        }

        // Validate AnyOfGroup
        void validateAnyOfGroup(const JsonVariant& j, const AnyOfGroup* group, bool& anyError)
        {
            if (!group) { return; } // failsafe just return

            bool found = false;
            
            for (size_t i = 0; group->fields[i] != nullptr; ++i) {
                const FieldBase* f = group->fields[i];
                if (j.containsKey(f->name)) {
                    found = true;
                    validateField(j, f, anyError);
                }
            }

            if (!found && group->flag == FieldFlag::Required) {
                GlobalLogger.Error(F("None of the AnyOfGroup fields present"));
                anyError = true;
            }
        }

        // Validate ModeSelector
        int evaluateModes(const JsonVariant& j, const ModeSelector* modes)
        {
            int matchedMode = -1;
            for (int i = 0; modes[i].name != nullptr; ++i)
            {
                const ModeSelector& mode = modes[i];
                bool modeValid = true;
                for (int c = 0; mode.conjunctions[c].fieldRef != nullptr; ++c)
                {
                    const ModeConjunctionDefine& conj = mode.conjunctions[c];
                    bool exists = false;
                    if (conj.fieldRef->type == FieldType::AnyOfGroup)
                    {
                        const AnyOfGroup* group =
                            static_cast<const AnyOfGroup*>(conj.fieldRef);

                        for (int g = 0; group->fields[g] != nullptr; ++g)
                        {
                            if (j.containsKey(group->fields[g]->name))
                            {
                                exists = true;
                                break;
                            }
                        }
                    }
                    else
                    {
                        exists = j.containsKey(conj.fieldRef->name);
                    }
                    if (conj.required != exists)
                    {
                        modeValid = false;
                        break;
                    }
                }
                if (modeValid)
                {
                    if (matchedMode != -1)
                    {
                        // multiple modes match -> ambiguous
                        return -2;
                    }
                    matchedMode = i;
                }
            }
            return matchedMode;
        }

        // Validate a full device
        int validateDevice(const JsonVariant& j, const JsonSchema::Device* devScheme, bool& anyError)
        {
            // 1. Check unknown fields
            for (const JsonPair& kv : j.as<JsonObject>()) {
                const char* key = kv.key().c_str();

                if (!isKnownField(key, devScheme->fields)) {
                    GlobalLogger.Warn(F("Unknown config field:"), key);
                    // as this should not render the json invalid
                    // anyError is not set
                }
            }

            // 2. Validate each field
            for (int i = 0; devScheme->fields[i] != nullptr; ++i) {
                const FieldBase* f = devScheme->fields[i];

                if (f->type == FieldType::AnyOfGroup) {
                    validateAnyOfGroup(j, static_cast<const AnyOfGroup*>(f), anyError);
                } else {
                    validateField(j, f, anyError);
                }
            }

            // 3. Evaluate modes
            int mode = evaluateModes(j, devScheme->modes);
            if (mode == -1) {
                GlobalLogger.Error(F("No valid configuration mode found"));
                anyError = true;
            } else if (mode == -2) {
                GlobalLogger.Error(F("Configuration matches multiple modes"));
                anyError = true;
            }
            return anyError ? -1 : mode;
        }

    }

}