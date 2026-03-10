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

#include <ArduinoJSON.h>
using json = JsonVariant;
 
namespace DALHAL {
    
    namespace JsonSchema {

        bool validateField(const json& j, const DALHAL::JsonSchema::FieldBase* field, std::string& error) {
            if (!field) return true;

            // check if the field exists in JSON
            if (!j.containsKey(field->name)) {
                if (field->flag == DALHAL::JsonSchema::FieldFlag::Required) {
                    error = "Required field missing: " + std::string(field->name);
                    return false;
                } else {
                    return true; // optional field can be missing
                }
            }

            const auto& value = j[field->name];

            switch (field->type) {
                case DALHAL::JsonSchema::FieldType::Int: {
                    auto f = static_cast<const DALHAL::JsonSchema::FieldInt*>(field);
                    if (!value.is<int32_t>()) {
                        error = std::string(field->name) + " must be an integer";
                        return false;
                    }
                    int v = value.as<int>();
                    if (v < f->minValue || v > f->maxValue) {
                        error = std::string(field->name) + " out of range";
                        return false;
                    }
                    break;
                }
                case DALHAL::JsonSchema::FieldType::UInt: {
                    auto f = static_cast<const DALHAL::JsonSchema::FieldUInt*>(field);
                    if (!value.is<uint32_t>()) {
                        error = std::string(field->name) + " must be unsigned integer";
                        return false;
                    }
                    uint32_t v = value.as<uint32_t>();
                    if (v < f->minValue || v > f->maxValue) {
                        error = std::string(field->name) + " out of range";
                        return false;
                    }
                    break;
                }
                case DALHAL::JsonSchema::FieldType::Float: {
                    auto f = static_cast<const DALHAL::JsonSchema::FieldFloat*>(field);
                    if (!value.as<float>()) {
                        error = std::string(field->name) + " must be a number";
                        return false;
                    }
                    float v = value.as<float>();
                    if (v < f->minValue || v > f->maxValue) {
                        error = std::string(field->name) + " out of range";
                        return false;
                    }
                    break;
                }
                case DALHAL::JsonSchema::FieldType::UID:
                case DALHAL::JsonSchema::FieldType::UID_Path:
                case DALHAL::JsonSchema::FieldType::Array:
                case DALHAL::JsonSchema::FieldType::Pin:
                    // TODO: handle these
                    break;
                case DALHAL::JsonSchema::FieldType::AnyOfGroup:
                    // handled recursively
                    break;
            }

            return true;
        }

        bool validateAnyOfGroup(const json& j, const DALHAL::JsonSchema::AnyOfGroup* group, std::string& error) {
            if (!group) return true;

            bool found = false;
            for (size_t i = 0; group->fields[i] != nullptr; ++i) {
                const auto* f = group->fields[i];
                if (j.containsKey(f->name)) {
                    found = true;
                    if (!validateField(j, f, error)) return false;
                }
            }

            if (!found && group->flag == DALHAL::JsonSchema::FieldFlag::Required) {
                error = "None of the AnyOfGroup fields present";
                return false;
            }

            return true;
        }

        bool validateMode(const json& j, const DALHAL::JsonSchema::ModeSelector* mode, std::string& error) {
            if (!mode) return true;

            for (size_t i = 0; mode->conjunctions[i].fieldRef != nullptr; ++i) {
                const auto& conj = mode->conjunctions[i];
                bool exists = j.containsKey(conj.fieldRef->name);
                if (conj.required && !exists) {
                    error = "Mode requirement failed: field " + std::string(conj.fieldRef->name) + " must exist";
                    return false;
                }
                if (!conj.required && exists) {
                    error = "Mode requirement failed: field " + std::string(conj.fieldRef->name) + " must NOT exist";
                    return false;
                }
            }

            return true;
        }

        bool validateDevice(const json& j, const DALHAL::JsonSchema::Device* device, uint32_t modeId, std::string& error) {
            if (!device) return false;

            // Validate fields
            for (size_t i = 0; device->fields[i] != nullptr; ++i) {
                const auto* f = device->fields[i];
                if (f->type == DALHAL::JsonSchema::FieldType::AnyOfGroup) {
                    if (!validateAnyOfGroup(j, static_cast<const DALHAL::JsonSchema::AnyOfGroup*>(f), error))
                        return false;
                } else {
                    if (!validateField(j, f, error)) return false;
                }
            }

            // Find mode
            const DALHAL::JsonSchema::ModeSelector* selectedMode = nullptr;
            for (size_t i = 0; device->modes[i] != nullptr; ++i) {
                if (device->modes[i]->modeId == modeId) {
                    selectedMode = device->modes[i];
                    break;
                }
            }

            if (selectedMode) {
                if (!validateMode(j, selectedMode, error)) return false;
            }

            return true;
        }

    }

}