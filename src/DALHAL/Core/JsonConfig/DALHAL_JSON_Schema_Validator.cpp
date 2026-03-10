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
#include <DALHAL/Support/DALHAL_Logger.h>

#include <ArduinoJSON.h>
using json = JsonVariant;
 
namespace DALHAL {

    namespace JsonSchema {

            
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
        bool validateStringField(const JsonVariant& value, const FieldString* f, std::string& error)
        {
            if (!value.is<const char*>()) {
                error = std::string(f->name) + " must be a string";
                return false;
            }
            const char* s = value.as<const char*>();
            if (f->maxLength > 0 && strlen(s) > f->maxLength) {
                error = std::string(f->name) + " exceeds maxLength";
                return false;
            }
            return true;
        }

        // Validate a single field
        bool validateField(const JsonObjectConst& j, const FieldBase* field, std::string& error)
        {
            if (!field) return true;

            // Handle required/optional
            if (!j.containsKey(field->name)) {
                if (field->flag == FieldFlag::Required) {
                    error = std::string("Required field missing: ") + field->name;
                    return false;
                }
                return true; // optional can be missing
            }

            JsonVariant value = j[field->name];

            switch (field->type) {
                case FieldType::Int: {
                    auto f = static_cast<const FieldInt*>(field);
                    if (!value.is<int>()) {
                        error = std::string(f->name) + " must be integer";
                        return false;
                    }
                    int v = value.as<int>();
                    if (v < f->minValue || v > f->maxValue) {
                        error = std::string(f->name) + " out of range";
                        return false;
                    }
                    break;
                }
                case FieldType::UInt: {
                    auto f = static_cast<const FieldUInt*>(field);
                    if (!value.is<unsigned int>()) {
                        error = std::string(f->name) + " must be unsigned int";
                        return false;
                    }
                    unsigned int v = value.as<unsigned int>();
                    if (v < f->minValue || v > f->maxValue) {
                        error = std::string(f->name) + " out of range";
                        return false;
                    }
                    break;
                }
                case FieldType::Float: {
                    auto f = static_cast<const FieldFloat*>(field);
                    if (!value.is<float>() && !value.is<double>() && !value.is<int>()) {
                        error = std::string(f->name) + " must be a number";
                        return false;
                    }
                    float v = value.as<float>();
                    if (v < f->minValue || v > f->maxValue) {
                        error = std::string(f->name) + " out of range";
                        return false;
                    }
                    break;
                }
                case FieldType::UID:
                case FieldType::UID_Path:
                case FieldType::Pin:
                case FieldType::Array:
                    // cast FieldString for UID / UID_Path / simple string fields
                    return validateStringField(value, static_cast<const FieldString*>(field), error);

                case FieldType::AnyOfGroup:
                    // handled by validateAnyOfGroup
                    break;
            }

            return true;
        }

        // Validate AnyOfGroup
        bool validateAnyOfGroup(const JsonObjectConst& j, const AnyOfGroup* group, std::string& error)
        {
            if (!group) return true;
            bool found = false;
            for (size_t i = 0; group->fields[i] != nullptr; ++i) {
                const FieldBase* f = group->fields[i];
                if (j.containsKey(f->name)) {
                    found = true;
                    if (!validateField(j, f, error)) return false;
                }
            }

            if (!found && group->flag == FieldFlag::Required) {
                error = "None of the AnyOfGroup fields present";
                return false;
            }

            return true;
        }

        // Validate ModeSelector
        int evaluateModes(const JsonObjectConst& j, const ModeSelector* modes)
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
        int validateDevice(const JsonObjectConst& j, const JsonSchema::Device* device, std::string& error)
        {
            // check for unknown fields
            for (JsonPairConst kv : j) {
                const char* key = kv.key().c_str();

                if (!isKnownField(key, device->fields)) {
                    GlobalLogger.Error(F("Unknown config field:"), key);
                }
            }
            // 1 Validate fields
            for (int i = 0; device->fields[i] != nullptr; ++i)
            {
                const FieldBase* f = device->fields[i];

                if (f->type == FieldType::AnyOfGroup)
                {
                    if (!validateAnyOfGroup(j, static_cast<const AnyOfGroup*>(f), error))
                        return -1;
                }
                else
                {
                    if (!validateField(j, f, error))
                        return -1;
                }
            }
            // 2 Detect mode
            int mode = evaluateModes(j, device->modes);
            if (mode == -1)
            {
                error = "No valid configuration mode found";
                return -1;
            }
            if (mode == -2)
            {
                error = "Configuration matches multiple modes";
                return -1;
            }
            return mode;
        }

    }

}