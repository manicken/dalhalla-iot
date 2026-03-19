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
#include <DALHAL/Core/Types/DALHAL_Value.h>

#include <ArduinoJSON.h>
#include <DALHAL/Support/ConvertHelper.h>

#include <DALHAL/Core/JsonConfig/CommonSchemes/DALHAL_CommonSchemes_Base.h>


//using json = JsonVariant;
 
namespace DALHAL {

    namespace JsonSchema {

        const char* ERROR_SOURCE_STR_VALIDATE_FROM_REGISTER = "JSON_Schema_Validator - validateFromRegister";
        const char* ERROR_SOURCE_STR_VALIDATE_FIELD = "JSON_Schema_Validator - validateField";

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
                } else if (f->type == FieldType::AllOfGroup) {
                    const AllOfGroup* group = static_cast<const AllOfGroup*>(f);

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
                GlobalLogger.setLastEntrySource("validateStringField");
                anyError = true;
                return;
            }
            ZeroCopyString zcStr = value.as<const char*>(); // wrap in ZeroCopyString for neat functions
            zcStr.Trim();
            size_t strLen = zcStr.Length(); // use of lenght here is fast
            if (strLen == 0) {
                GlobalLogger.Error(F("Field string cannot be empty:"), f->name);
                GlobalLogger.setLastEntrySource("validateStringField");
                anyError = true;
                return;
            }
            if (f->maxLength > 0 && strLen > f->maxLength) {
                GlobalLogger.Error(F("Field exceeds maxLength:"), f->name);
                GlobalLogger.setLastEntrySource("validateStringField");
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
                case FieldType::UID_Path: {
                    // cast FieldString for UID / UID_Path / simple string fields
                    validateStringField(value, static_cast<const FieldString*>(field), anyError);
                    return;
                }
                case FieldType::Array: {
                    auto f = static_cast<const FieldArray*>(field);
                    validateJsonObject(value, f->subtype, anyError);
                    return;
                }
                case FieldType::RegistryArray: {
                    auto f = static_cast<const FieldRegistryArray*>(field);
                    ValidateFromRegisterContext tmpContext(ValidateFromRegisterContext::State::Disabled);
                    validateFromRegister(value, f->subtypes, tmpContext, anyError);
                    break;
                }
                case FieldType::Object: {
                    auto f = static_cast<const FieldObject*>(field);
                    validateJsonObject(value, f->subtype, anyError);
                    break;
                }
                case FieldType::HexBytes: {
                    
                    bool anyErrorTemp = false;
                    validateStringField(value, static_cast<const FieldString*>(field), anyErrorTemp);
                    if (anyErrorTemp == true) {
                        anyError = true;
                        break; // no point of continue
                    }
                    auto f = static_cast<const FieldHexBytes*>(field);
                    const char* cStr = value.as<const char*>();
                    // TODO implement settings for delimiter enforcement
                    bool parseOk = Convert::HexToBytes(cStr, nullptr, f->byteCount);
                    if (parseOk == false) {
                        GlobalLogger.Error(F("validateField HexBytes parse error"));
                        GlobalLogger.setLastEntrySource(ERROR_SOURCE_STR_VALIDATE_FIELD);
                        anyError = true;
                    }
                    break;
                }
                case FieldType::AnyOfGroup:
                    // handled by validateAnyOfGroup
                    // as groups are not real fields
                    // just a "collection" of multiple fields
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

        void validateAllOfGroup(const JsonVariant& j, const AllOfGroup* group, bool& anyError)
        {
            if (!group) { return; }

            int foundCount = 0;
            int totalCount = 0;

            for (size_t i = 0; group->fields[i] != nullptr; ++i) {
                const FieldBase* f = group->fields[i];
                totalCount++;

                if (j.containsKey(f->name)) {
                    foundCount++;
                    validateField(j, f, anyError);
                }
            }

            // 🚨 core rule
            if (foundCount != 0 && foundCount != totalCount) {
                GlobalLogger.Error(F("AllOfGroup partially defined"));
                anyError = true;
                return;
            }

            if (foundCount == 0 && group->flag == FieldFlag::Required) {
                GlobalLogger.Error(F("Required AllOfGroup missing"));
                anyError = true;
            }
        }

        // Validate ModeSelector
        int evaluateModes(const JsonVariant& j, const ModeSelector* modes) {
            int matchedMode = -1;
            for (int i = 0; modes[i].name != nullptr; ++i) {
                const ModeSelector& mode = modes[i];
                bool modeValid = true;
                for (int c = 0; mode.conjunctions[c].fieldRef != nullptr; ++c) {
                    const ModeConjunctionDefine& conj = mode.conjunctions[c];
                    bool exists = false;
                    if (conj.fieldRef->type == FieldType::AnyOfGroup) {
                        const AnyOfGroup* group = static_cast<const AnyOfGroup*>(conj.fieldRef);

                        for (int g = 0; group->fields[g] != nullptr; ++g) {
                            if (j.containsKey(group->fields[g]->name)) {
                                exists = true;
                                break;
                            }
                        }
                    }
                    else if (conj.fieldRef->type == FieldType::AllOfGroup) {
                        const AllOfGroup* group = static_cast<const AllOfGroup*>(conj.fieldRef);
                        int found = 0;
                        int total = 0;

                        for (int g = 0; group->fields[g] != nullptr; ++g) {
                            total++;
                            if (j.containsKey(group->fields[g]->name)) {
                                found++;
                            }
                        }
                        // ✅ group "exists" ONLY if fully present
                        exists = (found == total && total > 0);
                    } else {
                        exists = j.containsKey(conj.fieldRef->name);
                    }
                    if (conj.required != exists) {
                        modeValid = false;
                        break;
                    }
                }
                if (modeValid) {
                    if (matchedMode != -1) {
                        // multiple modes match -> ambiguous
                        return -2;
                    }
                    matchedMode = i;
                }
            }
            return matchedMode;
        }

        bool evaluateConstraints_PrevalidateFields(const JsonVariant& j, const FieldConstraint& fcItem) {
            bool tempAnyError = false;
            validateField(j, fcItem.fieldA, tempAnyError);
            validateField(j, fcItem.fieldB, tempAnyError);
            if (tempAnyError) {
                GlobalLogger.Warn(F("both FieldConstraint fields must be valid"));
                GlobalLogger.setLastEntrySource("evaluateConstraints_PrevalidateFields");
                return false;
            }
            return true;
        }

        void evaluateConstraints(const JsonVariant& j, const FieldConstraint* constraints, bool& anyError) {
            if (constraints == nullptr) return;

            for (int i=0; constraints[i].type != FieldConstraint::Type::Void; ++i) {
                const FieldConstraint& fcItem = constraints[i];
                if (fcItem.fieldA->type != fcItem.fieldB->type) {
                    GlobalLogger.Warn(F("SchemaError - both FieldConstraint fields must be of the same type"));
                    GlobalLogger.setLastEntrySource("evaluateConstraints");
                    continue;
                }
                HALValue valA;
                HALValue valB;
                // now both are the same type so it wont matter which one we check the type against
                if (fcItem.fieldA->type == FieldType::UInt) {
                    // first validate so that the basic values are in range and that it's the correct type
                    if (evaluateConstraints_PrevalidateFields(j, fcItem) == false) {
                        continue;
                    }
                    auto fA = static_cast<const FieldUInt*>(fcItem.fieldA);
                    auto fB = static_cast<const FieldUInt*>(fcItem.fieldB);
                    valA = j.containsKey(fA->name)?j[fA->name].as<uint32_t>():fA->defaultValue;
                    valB = j.containsKey(fB->name)?j[fB->name].as<uint32_t>():fB->defaultValue;
                } else if (fcItem.fieldA->type == FieldType::Int) {
                    // first validate so that the basic values are in range and that it's the correct type
                    if (evaluateConstraints_PrevalidateFields(j, fcItem) == false) {
                        continue;
                    }
                    auto fA = static_cast<const FieldInt*>(fcItem.fieldA);
                    auto fB = static_cast<const FieldInt*>(fcItem.fieldB);
                    valA = j.containsKey(fA->name)?j[fA->name].as<int32_t>():fA->defaultValue;
                    valB = j.containsKey(fB->name)?j[fB->name].as<int32_t>():fB->defaultValue;

                } else if (fcItem.fieldA->type == FieldType::Float) {
                    // first validate so that the basic values are in range and that it's the correct type
                    if (evaluateConstraints_PrevalidateFields(j, fcItem) == false) {
                        continue;
                    }
                    auto fA = static_cast<const FieldFloat*>(fcItem.fieldA);
                    auto fB = static_cast<const FieldFloat*>(fcItem.fieldB);
                    valA = j.containsKey(fA->name)?j[fA->name].as<float>():fA->defaultValue;
                    valB = j.containsKey(fB->name)?j[fB->name].as<float>():fB->defaultValue;

                }/* else if (fcItem.fieldA->type == FieldType::Bool) { // could make sense in some situations
                    // keep it unimplemented for now
                } */else {
                    GlobalLogger.Warn(F("SchemaError - FieldConstraint fieldtype unsupported: "), FieldTypeToString(fcItem.fieldA->type));
                    GlobalLogger.setLastEntrySource("evaluateConstraints");
                    continue;
                }

                if (fcItem.type == FieldConstraint::Type::GreaterThan) {
                    if ((valA > valB) == false) {
                        anyError = true;
                        std::string err = fcItem.fieldA->name + std::string(" > ") + fcItem.fieldB->name;
                        GlobalLogger.Error(F("Constraint failed: "), err.c_str());
                    }
                } else if (fcItem.type == FieldConstraint::Type::GreaterThanOrEqual) {
                    if ((valA >= valB) == false) {
                        anyError = true;
                        std::string err = fcItem.fieldA->name + std::string(" >= ") + fcItem.fieldB->name;
                        GlobalLogger.Error(F("Constraint failed: "), err.c_str());
                    }
                } else if (fcItem.type == FieldConstraint::Type::LessThan) {
                    if ((valA < valB) == false) {
                        anyError = true;
                        std::string err = fcItem.fieldA->name + std::string(" < ") + fcItem.fieldB->name;
                        GlobalLogger.Error(F("Constraint failed: "), err.c_str());
                    }
                } else if (fcItem.type == FieldConstraint::Type::LessThanOrEqual) {
                    if ((valA <= valB) == false) {
                        anyError = true;
                        std::string err = fcItem.fieldA->name + std::string(" <= ") + fcItem.fieldB->name;
                        GlobalLogger.Error(F("Constraint failed: "), err.c_str());
                    }
                } else {
                    GlobalLogger.Error(F("SchemaError - Constraint type not found: "), FieldConstraintTypeToString(fcItem.type));
                }

            }
        }

        // Validate a complete JSON Object
        /*int*/ void validateJsonObject(const JsonVariant& j, const JsonSchema::JsonObjectScheme* jsonObjectScheme, bool& anyError)
        {
            // 1. Check unknown fields
            for (const JsonPair& kv : j.as<JsonObject>()) {
                const char* key = kv.key().c_str();

                if (!isKnownField(key, jsonObjectScheme->fields)) {
                    GlobalLogger.Warn(F("Unknown config field:"), key);
                    GlobalLogger.setLastEntrySource(jsonObjectScheme->typeName);
                    // as this should not render the json invalid
                    // anyError is not set
                }
            }

            // 2. Validate each field
            for (int i = 0; jsonObjectScheme->fields[i] != nullptr; ++i) {
                const FieldBase* f = jsonObjectScheme->fields[i];

                if (f->type == FieldType::AnyOfGroup) { // must validate this separate as it's a virtual group
                    validateAnyOfGroup(j, static_cast<const AnyOfGroup*>(f), anyError);
                } else if (f->type == FieldType::AllOfGroup) { // must validate this separate as it's a virtual group
                    validateAllOfGroup(j, static_cast<const AllOfGroup*>(f), anyError);
                } else {
                    validateField(j, f, anyError);
                }
            }

            // 3. Evaluate modes
            int mode = evaluateModes(j, jsonObjectScheme->modes);
            if (mode == -1) {
                GlobalLogger.Error(F("No valid configuration mode found"));
                anyError = true;
            } else if (mode == -2) {
                GlobalLogger.Error(F("Configuration matches multiple modes"));
                anyError = true;
            }

            // 4. Evaluate constraints
            evaluateConstraints(j, jsonObjectScheme->constraints, anyError);

            //return anyError ? -1 : mode; // dont think this is ever needed
        }

        // Validate the JSON array against the given device registry.
        void validateFromRegister(const JsonVariant& jsonArray, const Registry::Item* reg, ValidateFromRegisterContext& context, bool& anyError) {

            if (jsonArray.is<JsonArray>() == false) {
                GlobalLogger.Error(F("Json register field is not a array"));
                GlobalLogger.setLastEntrySource(ERROR_SOURCE_STR_VALIDATE_FROM_REGISTER);
                anyError = true;
                return; // can't continue here
            }
            const JsonArray& items = jsonArray.as<JsonArray>();
            if (items.size() == 0) {
                GlobalLogger.Error(F("Json register array is empty"));
                GlobalLogger.setLastEntrySource(ERROR_SOURCE_STR_VALIDATE_FROM_REGISTER);
                anyError = true;
                return; // can't continue here
            }
            uint32_t itemCount = items.size();
            context.Init(itemCount);
            
            for (uint32_t i = 0; i < itemCount; ++i) {
                const JsonVariant& jsonItem = items[i];
                context.SetDevice(i, false); // allways set to false first
                if (jsonItem.is<const char*>()) { continue; } // comment item
                if (DALHAL::Device::DisabledInJson(jsonItem)) { continue; } // disabled

                bool anyErrorTemp = false;
                validateField(jsonItem, &JsonSchema::typeField, anyErrorTemp); // this will internally emit errors to log
                if (anyErrorTemp == true) {
                    anyError = true;
                    continue; // skip the current json device
                }
                const char* type_cStr = jsonItem[DALHAL_COMMON_CFG_NAME_TYPE];
                const Registry::Item& regItem = Registry::GetItem(reg, type_cStr);
                if (regItem.typeName == nullptr) {
                    GlobalLogger.Error(F("could not find type:"),type_cStr);
                    GlobalLogger.setLastEntrySource(ERROR_SOURCE_STR_VALIDATE_FROM_REGISTER);
                    anyError = true;
                    continue; // skip the current json device
                }

                if (regItem.def == nullptr) {
                    GlobalLogger.Error(F("FATAL regitem.def == nullptr"));
                    GlobalLogger.setLastEntrySource(ERROR_SOURCE_STR_VALIDATE_FROM_REGISTER);
                    anyError = true;
                    continue; // skip the current json device
                }
                
                if (regItem.def->jsonSchema == nullptr) {
                    // TODO remove use of obsolete function Verify_JSON_Function
                    // can only be done when all devices use json schema
                    if (regItem.def->Verify_JSON_Function == nullptr) {
                    GlobalLogger.Error(F("FATAL regItem.def->jsonSchema == nullptr"));
                    GlobalLogger.setLastEntrySource(ERROR_SOURCE_STR_VALIDATE_FROM_REGISTER);
                    anyError = true;
                    continue; // skip the current json device
                    }
                    // TODO remove use of obsolete function Verify_JSON_Function
                    // can only be done when all devices use json schema
                    if (jsonItem.containsKey(DALHAL_COMMON_CFG_NAME_UID) == false) {
                        GlobalLogger.Error(F("uid field missing"));
                        GlobalLogger.setLastEntrySource(ERROR_SOURCE_STR_VALIDATE_FROM_REGISTER);
                        anyError = true;
                        continue; // skip the current json device
                    }
                    bool uidFoundError = false;
                    validateStringField(jsonItem[DALHAL_COMMON_CFG_NAME_UID], &uidFieldRequired, uidFoundError);
                    if (uidFoundError) {
                        anyError = true;
                        continue;
                    }
                    if (regItem.def->Verify_JSON_Function(jsonItem) == false) {
                        anyError = true;
                        continue;
                    } else {
                        context.SetDevice(i, true);
                    }
                    continue;
                }
                bool validatedJsonObjectAnyError = false;
                validateJsonObject(jsonItem, regItem.def->jsonSchema, validatedJsonObjectAnyError);
                if (validatedJsonObjectAnyError) {
                    anyError = true;
                } else {
                    context.SetDevice(i, true);
                }
            }
        }

    }

}