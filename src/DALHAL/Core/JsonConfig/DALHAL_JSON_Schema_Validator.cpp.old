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

#include "DALHAL_JSON_Schema_Validator.h"
#include "DALHAL/Core/Manager/DALHAL_GPIO_Manager.h"
#include <DALHAL/Support/DALHAL_Logger.h>
#include <DALHAL/Core/Types/DALHAL_ZeroCopyString.h>
#include <DALHAL/Core/Types/DALHAL_Value.h>

#include <ArduinoJSON.h>
#include <DALHAL/Support/ConvertHelper.h>

#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Base.h>

#include <math.h>
#include <cstdint>

#include <DALHAL/Core/JsonConfig/DALHAL_ArduinoJSON_ext.h>


#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_TypeBase.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/Groups/DALHAL_JSON_Schema_FieldsGroup.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/Groups/DALHAL_JSON_Schema_AllOfFieldsGroup.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/Groups/DALHAL_JSON_Schema_OneOfFieldsGroup.h>
#include <DALHAL/Core/JsonConfig/Types/Root/DALHAL_JSON_Schema_JsonObjectSchema.h>

#include <DALHAL/Core/JsonConfig/Types/Structures/DALHAL_JSON_Schema_ArrayOfObjects.h>
#include <DALHAL/Core/JsonConfig/Types/Structures/DALHAL_JSON_Schema_ArrayOfPrimitives.h>
#include <DALHAL/Core/JsonConfig/Types/Primitives/DALHAL_JSON_Schema_Bool.h>
#include <DALHAL/Core/JsonConfig/Types/Primitives/DALHAL_JSON_Schema_Float.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/DALHAL_JSON_Schema_HardwarePin.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/DALHAL_JSON_Schema_HardwarePinOrVirtualPin.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/String/DALHAL_JSON_Schema_StringHexBytes.h>
#include <DALHAL/Core/JsonConfig/Types/Primitives/DALHAL_JSON_Schema_Int.h>
#include <DALHAL/Core/JsonConfig/Types/Primitives/DALHAL_JSON_Schema_Number.h>
#include <DALHAL/Core/JsonConfig/Types/Structures/DALHAL_JSON_Schema_Object.h>
#include <DALHAL/Core/JsonConfig/Types/Structures/DALHAL_JSON_Schema_ArrayOfRegistryItems.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/String/DALHAL_JSON_Schema_StringAnyOfArrayConstrained.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/String/DALHAL_JSON_Schema_StringAnyOfByFuncConstrained.h>
#include <DALHAL/Core/JsonConfig/Types/Primitives/DALHAL_JSON_Schema_String.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/String/DALHAL_JSON_Schema_StringSizeConstrained.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/String/DALHAL_JSON_Schema_StringUID.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/String/DALHAL_JSON_Schema_StringUID_Path.h>
#include <DALHAL/Core/JsonConfig/Types/Primitives/DALHAL_JSON_Schema_UInt.h>

#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_PrimitiveTypeFlags.h>

#include <DALHAL/Core/JsonConfig/Types/Root/DALHAL_JSON_Schema_FieldConstraint.h>
#include <DALHAL/Core/JsonConfig/Types/Root/DALHAL_JSON_Schema_ModeSelector.h>
 
namespace DALHAL {

    namespace JsonSchema {

        // this is only a helper/support function and do not use anyError
        // as this could be just defined as a warning, depending on strict level requirements
        bool isUnknownField(const char* key, const SchemaTypeBase* const* fields)
        {
            for (int i = 0; fields[i] != nullptr; i++) {
                const SchemaTypeBase* f = fields[i];

                if (f->type == FieldType::OneOfFieldsGroup) {
                    const SchemaOneOfFieldsGroup* group = static_cast<const SchemaOneOfFieldsGroup*>(f);

                    for (int g = 0; group->fields[g] != nullptr; g++) {
                        if (group->fields[g]->name == nullptr) {
                            GlobalLogger.Warn(F("OneOfGroup - group->fields[g]->name == nullptr @ key: "), key);
                            continue;
                        }
                        if (strcmp(key, group->fields[g]->name) == 0)
                            return false; // isUnknownField
                    }
                } else if (f->type == FieldType::AllOfFieldsGroup) {
                    const SchemaAllOfFieldsGroup* group = static_cast<const SchemaAllOfFieldsGroup*>(f);

                    for (int g = 0; group->fields[g] != nullptr; g++) {
                        if (group->fields[g]->name == nullptr) {
                            GlobalLogger.Warn(F("AllOfGroup - group->fields[g]->name == nullptr @ key: "), key);
                            continue;
                        }
                        if (strcmp(key, group->fields[g]->name) == 0)
                            return false; // isUnknownField
                    }
                } else if (f->type == FieldType::FieldsGroup) {
                    const SchemaFieldsGroup* group = static_cast<const SchemaFieldsGroup*>(f);
                    if (!isUnknownField(key, group->fields)) { // recurse into the subgroup
                        return false;
                    }
                }
                else {
                    if (f->name == nullptr) {
                        GlobalLogger.Warn(F("f->name == nullptr @ key: "), key);
                        continue;
                    }
                    //Serial.println(key);
                    //Serial.print(FieldTypeToString(f->type));
                    //Serial.println(f->name);
                    if (strcmp(key, f->name) == 0)
                        return false; // isUnknownField
                }
            }

            return true; // isUnknownField
        }

        // Helper to validate FieldString / FieldUID
        void validateStringField(const JsonVariant& value, const char* sourceObjTypeName, const SchemaString* f, bool& anyError)
        {
            if (!value.is<const char*>()) {
                std::string errMsg = f->name;
                errMsg += " @ "; errMsg += '['; errMsg += sourceObjTypeName; errMsg += ']';
                GlobalLogger.Error(F("Field must be a string:"), errMsg.c_str());

                anyError = true;
                return;
            }
            const char* value_cStr = value.as<const char*>();
            ZeroCopyString zcStr = value_cStr; // wrap in ZeroCopyString for neat functions
            zcStr.Trim();
            unsigned int strLen = zcStr.Length(); // use of lenght here is fast
            if (f->type == FieldType::String || f->type == FieldType::StringUID_Path) {
                if (strLen == 0) {
                    anyError = true;
                    std::string errMsg = f->name;
                    errMsg += " @ "; errMsg += '['; errMsg += sourceObjTypeName; errMsg += ']';
                    GlobalLogger.Error(F("Basic String is empty: "), errMsg.c_str());
                }
                return;
            }
            if (f->type == FieldType::StringSizeConstrained || f->type == FieldType::StringUID || f->type == FieldType::StringHexBytes ) {
                const SchemaStringSizeConstrained* fssc = static_cast<const SchemaStringSizeConstrained*>(f);
                if (strLen < fssc->minLength) {
                    std::string errMsg = std::to_string((unsigned int)fssc->minLength) + "): ";
                    errMsg += f->name;
                    errMsg += " @ "; errMsg += '['; errMsg += sourceObjTypeName; errMsg += ']';
                    GlobalLogger.Error(F("String shorter than minLength("), errMsg.c_str());
                    anyError = true;
                    return;
                }
                if (fssc->maxLength > 0 && strLen > fssc->maxLength) {
                    std::string errMsg = std::to_string((unsigned int)fssc->maxLength) + "): ";
                    errMsg += f->name;
                    errMsg += " @ "; errMsg += '['; errMsg += sourceObjTypeName; errMsg += ']';
                    GlobalLogger.Error(F("String exceeds maxLength("), errMsg.c_str());
                    anyError = true;
                    return;
                }
            } else if (f->type == FieldType::StringAnyOfByFuncConstrained) {
                if (strLen == 0) { // fast fail
                    anyError = true;
                    std::string errMsg;
                    errMsg += f->name;
                    errMsg += " @ "; errMsg += '['; errMsg += sourceObjTypeName; errMsg += ']';
                    GlobalLogger.Error(F("String is empty @ StringAnyOfByFuncConstrained mode: "), errMsg.c_str());
                    return;
                }
                const SchemaStringAnyOfByFuncConstrained* fsaobfc = static_cast<const SchemaStringAnyOfByFuncConstrained*>(f);
                if (fsaobfc->validate(fsaobfc->ctx, value_cStr) == false) {
                    anyError = true;
                    std::string errMsg;
                    errMsg += f->name;
                    errMsg += " @ "; errMsg += '['; errMsg += sourceObjTypeName; errMsg += ']';
                    GlobalLogger.Error(F("Invalid value for field: "), errMsg.c_str());
                }
            } // else other string based types that do inherit from stringbase but take care of their own validation
            
        }

        // Validate a single field
        void validateField(const JsonVariant& j, const char* sourceObjTypeName, const SchemaTypeBase* field, bool& anyError)
        {
            if (sourceObjTypeName == nullptr) {
                 std::string errMsg;
                serializeCollapsed(j, errMsg);
                GlobalLogger.Error(F(" SCHEMA ERROR - nullptr error"), errMsg.c_str());
                sourceObjTypeName = "nullptr error";
            }
            if (!field) { return; } // failsafe just return

            // Handle required/optional
            if (!j.containsKey(field->name)) {
                if (field->policy == FieldPolicy::Required) {
                    std::string errMsg = field->name; 
                    errMsg += " @ "; errMsg += '['; errMsg += sourceObjTypeName; errMsg += ']';
                    errMsg += ' ';
                    serializeCollapsed(j, errMsg);
                    GlobalLogger.Error(F("Required field missing: "), errMsg.c_str());
                    
                    anyError = true;
                    return;
                }
                return; // optional can be missing
            }

            JsonVariant value = j[field->name];

            switch (field->type) {
                case FieldType::Bool: {
                    if (!value.is<bool>()) {
                        std::string errStr = field->name; errStr += " @ ";
                        serializeCollapsed(j, errStr);
                        GlobalLogger.Error(F(" must be a boolean (bool): "), errStr.c_str());

                        anyError = true;
                        return;
                    }
                    break;
                }
                case FieldType::Int: {
                    
                    if (!value.is<int>()) {
                        std::string errStr = field->name; errStr += " @ ";
                        serializeCollapsed(j, errStr);
                        GlobalLogger.Error(F(" must be a integer: "), errStr.c_str());

                        anyError = true;
                        return;
                    }
                    auto f = static_cast<const SchemaInt*>(field);
                    int v = value.as<int>();
                    if ((v < f->minValue) || ((f->maxValue != 0) && (v > f->maxValue))) { // if maxValue == 0 then the value can be anything
                        std::string errStr = field->name; errStr += " @ ";
                        serializeCollapsed(j, errStr);
                        GlobalLogger.Error(F(" int out of range: "), errStr.c_str());

                        anyError = true;
                        return;
                    }
                    break;
                }
                case FieldType::UInt: {
                    auto f = static_cast<const SchemaUInt*>(field);
                    if (!value.is<unsigned int>()) {
                        std::string errStr = field->name; errStr += " @ ";
                        serializeCollapsed(j, errStr);
                        GlobalLogger.Error(F(" must be unsigned int: "), errStr.c_str());

                        anyError = true;
                        return;
                    }
                    unsigned int v = value.as<unsigned int>();
                    if (((f->minValue != 0) && (v < f->minValue)) || ((f->maxValue != 0) && (v > f->maxValue))) { // if maxValue == 0 then the value can be anything
                        std::string errStr = field->name; errStr += " @ ";
                        serializeCollapsed(j, errStr);
                        GlobalLogger.Error(F(" uint out of range: "), errStr.c_str());

                        anyError = true;
                        return;
                    }
                    break;
                }
                case FieldType::Float: {
                    auto f = static_cast<const SchemaFloat*>(field);
                    if (!value.is<float>() && !value.is<double>() && !value.is<int>()) {
                        std::string errStr = field->name; errStr += " @ ";
                        serializeCollapsed(j, errStr);
                        GlobalLogger.Error(F(" must be a number: "), errStr.c_str());

                        anyError = true;
                        return;
                    }
                    float v = value.as<float>();
                    if (((isnan(f->minValue) == false) && (v < f->minValue)) || (((isnan(f->maxValue) == false) && (v > f->maxValue)))) { // if maxValue == 0 then the value can be anything
                        std::string errStr = field->name; errStr += " @ ";
                        serializeCollapsed(j, errStr);
                        GlobalLogger.Error(F(" float out of range: "), errStr.c_str());

                        anyError = true;
                        return;
                    }
                    break;
                }
                case FieldType::HardwarePin: {
                    auto f = static_cast<const SchemaHardwarePin*>(field);
                    if (value.is<int8_t>() == false) {
                        std::string errStr = field->name; errStr += " @ ";
                        serializeCollapsed(j, errStr);
                        GlobalLogger.Error(F("Harware Pin is not a valid value: "), errStr.c_str());
                    }
                    
                    int8_t pin = value.as<int8_t>();
                    if (pin < 0) {
                        // disabled pin use, just ignore for now,
                        // but should be required when this field is required
                        // otherwise it would lead to invalid config
                        // but the disabled field must also be taken into consideration
                        // as when disabled == true that is the only time this field do not need full valiation
                        // think this could be solved by a special flag
                        // or as a schema validator requirement that the disabled status is allways present on all field checks
                        // that way each validation can take that into consideration
                        // so in this case we could just do abs on the pin and validate the func/mode but not the collision
                        return; 
                    }
                    DALHAL_GPIO_MGR_PINFUNC_TYPE fMode = f->mode;
                    GPIO_manager::CheckPinResult res = GPIO_manager::CheckIfPinAvailableAndIsFree_ThenReserve(pin, fMode);
                    if (res != GPIO_manager::CheckPinResult::Success) {
                        GPIO_manager::CheckPinResultError errMsg = GPIO_manager::GetCheckPinResultError(res, pin, fMode);
                        std::string errStr = errMsg.msg + ", fName=" + field->name; errStr += " @ ";
                        serializeCollapsed(j, errStr);
                        GlobalLogger.Error(F(errMsg.baseMsg), errStr.c_str());
                        anyError = true;
                    }
                    return;
                }
                case FieldType::HardwarePinOrVirtualPin: {
                    auto f = static_cast<const SchemaHardwarePinOrVirtualPin*>(field); // TODO implement
                    if (value.is<const char*>()) {
                        bool anyErrorTemp = false;

                        validateStringField(value, sourceObjTypeName, static_cast<const SchemaString*>(field), anyErrorTemp);
                        if (anyErrorTemp == true) {
                            anyError = true;
                            return;
                        }
                        // TODO implement functionality to check if "virtual pin device" exists
                    } else {

                    }
                    return;
                }
                case FieldType::String:
                case FieldType::StringAnyOfArrayConstrained:
                case FieldType::StringAnyOfByFuncConstrained:
                case FieldType::StringSizeConstrained:
                case FieldType::StringUID:
                case FieldType::StringUID_Path: { // TODO make own validator for UID_Path as it need special tests except to be a simple string
                    // cast FieldString for UID / UID_Path / simple string fields
                    validateStringField(value, sourceObjTypeName, static_cast<const SchemaString*>(field), anyError);
                    return;
                }
                case FieldType::Object: {
                    validateJsonObject(value, field->name, static_cast<const SchemaObject*>(field)->subtype, anyError);
                    break;
                }
                case FieldType::ArrayOfObjects: {
                    validateJsonArray(value, static_cast<const SchemaArrayOfObjects*>(field), anyError);
                    return;
                }
                case FieldType::ArrayOfPrimitives: {
                    validateJsonArrayPrimitive(value, static_cast<const SchemaArrayOfPrimitives*>(field), anyError);
                    return;
                }
                case FieldType::ArrayOfRegistryItems: {
                    validateFromRegister(value, static_cast<const SchemaArrayOfRegistryItems*>(field)->subtypes, anyError);
                    break;
                }
                
                case FieldType::StringHexBytes: {
                    
                    bool anyErrorTemp = false;
                    validateStringField(value, sourceObjTypeName, static_cast<const SchemaString*>(field), anyErrorTemp);
                    if (anyErrorTemp == true) {
                        anyError = true;
                        break; // no point of continue
                    }
                    auto f = static_cast<const SchemaStringHexBytes*>(field);
                    const char* cStr = value.as<const char*>();
                    // TODO implement settings for delimiter enforcement
                    bool parseOk = Convert::HexToBytes(cStr, nullptr, f->byteCount);
                    if (parseOk == false) {
                        std::string errMsg = "f->byteCount:" + std::to_string(f->byteCount);
                        errMsg += " @ "; errMsg += cStr;
                        GlobalLogger.Error(F("validateField HexBytes parse error: "), errMsg.c_str());

                        anyError = true;
                    }
                    break;
                }
                case FieldType::Number: {
                    auto f = static_cast<const SchemaNumber*>(field);
                    if ((f->primitiveTypeFlags & PrimitiveTypeFlags::AllowFloat) && value.is<float>()) {
                        // accept as float
                        return;
                    } else if ((f->primitiveTypeFlags & PrimitiveTypeFlags::AllowInt) && value.is<int>()) {
                        // accept as signed int
                        return;
                    } else if ((f->primitiveTypeFlags & PrimitiveTypeFlags::AllowUInt) && value.is<unsigned int>()) {
                        // accept as unsigned int
                        return;
                    } else {
                        // reject
                        GlobalLogger.Error(F("validateField Number parse error: "), f->name);

                        anyError = true;
                        return;
                    }
                        
                }
                // virtual fields not handled here
                case FieldType::OneOfFieldsGroup:
                case FieldType::AllOfFieldsGroup:
                case FieldType::FieldsGroup:
                    break;
            }
        }

        void validateGroup(const JsonVariant& j, const char* sourceObjTypeName, const SchemaFieldsGroup* group, bool& anyError) {
            if (!group) { return; } // failsafe just return

            for (size_t i = 0; group->fields[i] != nullptr; ++i) {
                const SchemaTypeBase* f = group->fields[i];

                if (f->type == FieldType::OneOfFieldsGroup) { // must validate this separate as it's a virtual group
                    validateOneOfGroup(j, group->name?group->name:sourceObjTypeName, static_cast<const SchemaOneOfFieldsGroup*>(f), anyError);
                } else if (f->type == FieldType::AllOfFieldsGroup) { // must validate this separate as it's a virtual group
                    validateAllOfGroup(j, group->name?group->name:sourceObjTypeName, static_cast<const SchemaAllOfFieldsGroup*>(f), anyError);
                } else if (f->type == FieldType::FieldsGroup) {
                    validateGroup(j, sourceObjTypeName, static_cast<const SchemaFieldsGroup*>(f), anyError);
                } else {
                    validateField(j, group->name?group->name:sourceObjTypeName, f, anyError);
                }
            }

        }

        // Validate OneOfGroup
        void validateOneOfGroup(const JsonVariant& j, const char* fieldName, const SchemaOneOfFieldsGroup* group, bool& anyError)
        {
            if (!group) { return; } // failsafe just return

            bool found = false;
            
            for (size_t i = 0; group->fields[i] != nullptr; ++i) {
                const SchemaTypeBase* f = group->fields[i];
                if (j.containsKey(f->name)) {
                    found = true;
                    validateField(j, group->name, f, anyError);
                }
            }

            if (!found && group->policy == FieldPolicy::Required) {
                std::string errMsg = group->name;
                errMsg += " @ "; errMsg += fieldName;
                std::string jsonStr;
                serializeCollapsed(j, jsonStr);
                errMsg += jsonStr;
                GlobalLogger.Error(F("None of the OneOfGroup fields present: "), errMsg.c_str());
                anyError = true;
            }
        }

        void validateAllOfGroup(const JsonVariant& j, const char* fieldName, const SchemaAllOfFieldsGroup* group, bool& anyError)
        {
            if (!group) { return; }

            int foundCount = 0;
            int totalCount = 0;

            for (size_t i = 0; group->fields[i] != nullptr; ++i) {
                const SchemaTypeBase* f = group->fields[i];
                totalCount++;

                if (j.containsKey(f->name)) {
                    foundCount++;
                    validateField(j, group->name, f, anyError);
                }
            }

            // 🚨 core rule
            if (foundCount != 0 && foundCount != totalCount) {
                std::string errMsg = group->name;
                errMsg += " @ "; errMsg += fieldName;
                GlobalLogger.Error(F("AllOfGroup partially defined: "), errMsg.c_str());
                anyError = true;
                return;
            }

            if (foundCount == 0 && group->policy == FieldPolicy::Required) {
                std::string errMsg = group->name;
                errMsg += " @ "; errMsg += fieldName;
                GlobalLogger.Error(F("Required AllOfGroup missing: "), errMsg.c_str());
                anyError = true;
            }
        }

        // Validate ModeSelector
        /*int evaluateModes(const JsonVariant& j, const ModeSelector* modes) {
            int matchedMode = -1;
            for (int i = 0; modes[i].name != nullptr; ++i) {
                const ModeSelector& mode = modes[i];
                bool modeValid = true;
                for (int c = 0; mode.conjunctions[c].fieldRef != nullptr; ++c) {
                    const ModeConjunctionDefine& conj = mode.conjunctions[c];
                    bool exists = false;
                    if (conj.fieldRef->type == FieldType::OneOfFieldsGroup) {
                        const SchemaOneOfFieldsGroup* group = static_cast<const SchemaOneOfFieldsGroup*>(conj.fieldRef);

                        for (int g = 0; group->fields[g] != nullptr; ++g) {
                            if (j.containsKey(group->fields[g]->name)) {
                                exists = true;
                                break;
                            }
                        }
                    }
                    else if (conj.fieldRef->type == FieldType::AllOfFieldsGroup) {
                        const SchemaAllOfFieldsGroup* group = static_cast<const SchemaAllOfFieldsGroup*>(conj.fieldRef);
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
        }*/

        /*bool evaluateConstraints_PrevalidateFields(const JsonVariant& j, const char* sourceObjectTypeName, const FieldConstraint& fcItem) {
            bool tempAnyError = false;
            validateField(j, sourceObjectTypeName, fcItem.fieldA, tempAnyError);
            validateField(j, sourceObjectTypeName, fcItem.fieldB, tempAnyError);
            if (tempAnyError) {
                GlobalLogger.Warn(F("both FieldConstraint fields must be valid"));

                return false;
            }
            return true;
        }

        void evaluateConstraints(const JsonVariant& j, const char* sourceObjectTypeName, const FieldConstraint* constraints, bool& anyError) {
            if (constraints == nullptr) return;

            for (int i=0; constraints[i].type != FieldConstraint::Type::Void; ++i) {
                const FieldConstraint& fcItem = constraints[i];
                if (fcItem.fieldA->type != fcItem.fieldB->type) {
                    GlobalLogger.Warn(F("SchemaError - both FieldConstraint fields must be of the same type"), sourceObjectTypeName);

                    continue;
                }
                HALValue valA;
                HALValue valB;
                // now both are the same type so it wont matter which one we check the type against
                if (fcItem.fieldA->type == FieldType::UInt) {
                    // first validate so that the basic values are in range and that it's the correct type
                    if (evaluateConstraints_PrevalidateFields(j, sourceObjectTypeName, fcItem) == false) {
                        continue;
                    }
                    auto fA = static_cast<const SchemaUInt*>(fcItem.fieldA);
                    auto fB = static_cast<const SchemaUInt*>(fcItem.fieldB);
                    valA = j.containsKey(fA->name)?j[fA->name].as<uint32_t>():fA->defaultValue;
                    valB = j.containsKey(fB->name)?j[fB->name].as<uint32_t>():fB->defaultValue;
                } else if (fcItem.fieldA->type == FieldType::Int) {
                    // first validate so that the basic values are in range and that it's the correct type
                    if (evaluateConstraints_PrevalidateFields(j, sourceObjectTypeName, fcItem) == false) {
                        continue;
                    }
                    auto fA = static_cast<const SchemaInt*>(fcItem.fieldA);
                    auto fB = static_cast<const SchemaInt*>(fcItem.fieldB);
                    valA = j.containsKey(fA->name)?j[fA->name].as<int32_t>():fA->defaultValue;
                    valB = j.containsKey(fB->name)?j[fB->name].as<int32_t>():fB->defaultValue;

                } else if (fcItem.fieldA->type == FieldType::Float) {
                    // first validate so that the basic values are in range and that it's the correct type
                    if (evaluateConstraints_PrevalidateFields(j, sourceObjectTypeName, fcItem) == false) {
                        continue;
                    }
                    auto fA = static_cast<const SchemaFloat*>(fcItem.fieldA);
                    auto fB = static_cast<const SchemaFloat*>(fcItem.fieldB);
                    valA = j.containsKey(fA->name)?j[fA->name].as<float>():fA->defaultValue;
                    valB = j.containsKey(fB->name)?j[fB->name].as<float>():fB->defaultValue;

                } else {
                    GlobalLogger.Warn(F("SchemaError - FieldConstraint fieldtype unsupported: "), FieldTypeToString(fcItem.fieldA->type));

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
        }*/

        // Validate a complete JSON Object
        void validateJsonObject(const JsonVariant& j, const char* fieldName, const JsonSchema::JsonObjectSchema* jsonObjectSchema, bool& anyError)
        {
            if (jsonObjectSchema == nullptr) { return; }// allow any content

            if (j.size() == 0) {
                if (jsonObjectSchema->emptyPolicy == EmptyPolicy::Warn) {
                    std::string errStr = jsonObjectSchema->typeName;
                    errStr += " @ ";errStr += fieldName;
                    GlobalLogger.Warn(F("JsonObject is empty: "), errStr.c_str());
                    
                } else if (jsonObjectSchema->emptyPolicy == EmptyPolicy::Error) {
                    anyError = true;
                    std::string errStr = jsonObjectSchema->typeName;
                    errStr += " @ ";errStr += fieldName;
                    GlobalLogger.Error(F("JsonObject is empty: "), errStr.c_str());
                   
                } /*else if (jsonObjectSchema->emptyPolicy == EmptyPolicy::Ignore) {
                    // simply do nothing
                }*/
                return;
            }
            
            // 1. Check unknown fields
            for (const JsonPair& kv : j.as<JsonObject>()) {
                const char* key = kv.key().c_str();
                if (key == nullptr) {
                    GlobalLogger.Warn(F("key == nullptr: "), jsonObjectSchema->typeName);
                    continue;
                }
                if (jsonObjectSchema->fields == nullptr) {
                    GlobalLogger.Warn(F("jsonObjectSchema->fields == nullptr: "), key);
                    continue;
                }

                if (isUnknownField(key, jsonObjectSchema->fields)) {
                    if (jsonObjectSchema->unknownFieldPolicy == UnknownFieldPolicy::Ignore) {
                        continue;
                    } else if (jsonObjectSchema->unknownFieldPolicy == UnknownFieldPolicy::Warn) {
                        std::string errMsg = key;
                        errMsg += " @ "; errMsg += jsonObjectSchema->typeName;
                        GlobalLogger.Warn(F("Unknown config field: "), errMsg.c_str());
                    } else if (jsonObjectSchema->unknownFieldPolicy == UnknownFieldPolicy::Error) {
                        std::string errMsg = key;
                        errMsg += " @ "; errMsg += jsonObjectSchema->typeName;
                        GlobalLogger.Error(F("Unknown config field: "), errMsg.c_str());
                        anyError = true;
                    }                    
                    
                    // as this should not render the json invalid
                    // anyError is not set
                }
            }

            // 2. Validate each field
            for (int i = 0; jsonObjectSchema->fields[i] != nullptr; ++i) {
                const SchemaTypeBase* f = jsonObjectSchema->fields[i];

                if (f->type == FieldType::OneOfFieldsGroup) { // must validate this separate as it's a virtual group
                    validateOneOfGroup(j, jsonObjectSchema->typeName, static_cast<const SchemaOneOfFieldsGroup*>(f), anyError);
                } else if (f->type == FieldType::AllOfFieldsGroup) { // must validate this separate as it's a virtual group
                    validateAllOfGroup(j, jsonObjectSchema->typeName, static_cast<const SchemaAllOfFieldsGroup*>(f), anyError);
                } else if (f->type == FieldType::FieldsGroup) {
                    validateGroup(j, jsonObjectSchema->typeName, static_cast<const SchemaFieldsGroup*>(f), anyError);
                } else {
                    validateField(j, jsonObjectSchema->typeName, f, anyError);
                }
            }

            // 3. Evaluate modes if available
            if (jsonObjectSchema->modes) {
                int mode = evaluateModes(j, jsonObjectSchema->modes);
                if (mode == -1) {
                    std::string errInfoMsg = "["; errInfoMsg += jsonObjectSchema->typeName; errInfoMsg += "] ";
                    serializeCollapsed(j, errInfoMsg);
                    GlobalLogger.Error(F("No valid configuration mode found @ "), errInfoMsg.c_str());

                    anyError = true;
                } else if (mode == -2) {
                    std::string errInfoMsg = "["; errInfoMsg += jsonObjectSchema->typeName; errInfoMsg += "] ";
                    serializeCollapsed(j, errInfoMsg);
                    GlobalLogger.Error(F("Configuration matches multiple modes @ "), errInfoMsg.c_str());
                    anyError = true;
                }
            }

            // 4. Evaluate constraints if available
            if (jsonObjectSchema->constraints) {
                evaluateConstraints(j, jsonObjectSchema->typeName, jsonObjectSchema->constraints, anyError);
            }
            //return anyError ? -1 : mode; // dont think this is ever needed
        }

        void validateJsonArray(const JsonVariant& j, const SchemaArrayOfObjects* field, bool& anyError)
        {
            if (!j.is<JsonArray>()) {
                GlobalLogger.Error(F("Field is not an array"), field->name);
                anyError = true;
                return;
            }

            const JsonArray& arr = j.as<JsonArray>();

            // Empty policy (THIS is where it belongs for arrays)
            if (arr.size() == 0) {
                if (field->emptyPolicy == EmptyPolicy::Error) {
                    GlobalLogger.Error(F("Array is empty"), field->name);
                    anyError = true;
                } else if (field->emptyPolicy == EmptyPolicy::Warn) {
                    GlobalLogger.Warn(F("Array is empty"), field->name);
                }
                return;
            }

            // Validate each element
            for (JsonVariant item : arr) {
                if (item.is<const char*>()) continue; // comment item
                validateJsonObject(item, field->name, field->subtype, anyError);
            }
        }

        void validateJsonArrayPrimitive(const JsonVariant& j, const JsonSchema::SchemaArrayOfPrimitives* field, bool& anyError)
        {
            using namespace JsonSchema;

            if (!j.is<JsonArray>()) {
                GlobalLogger.Error(F("Field is not an array"), field->name);
                anyError = true;
                return;
            }

            const JsonArray& arr = j.as<JsonArray>();

            // Empty policy
            if (arr.size() == 0) {
                if (field->emptyPolicy == EmptyPolicy::Error) {
                    GlobalLogger.Error(F("Array is empty"), field->name);
                    anyError = true;
                } else if (field->emptyPolicy == EmptyPolicy::Warn) {
                    GlobalLogger.Warn(F("Array is empty"), field->name);
                }
                return;
            }

            // Validate each element against primitiveTypeFlags
            for (JsonVariant item : arr) {
                bool valid = false;

                if (item.is<int>()) {
                    valid = field->primitiveTypeFlags & PrimitiveTypeFlags::AllowInt;
                } else if (item.is<unsigned int>()) {
                    valid = field->primitiveTypeFlags & PrimitiveTypeFlags::AllowUInt;
                } else if (item.is<float>() || item.is<double>()) {
                    valid = field->primitiveTypeFlags & PrimitiveTypeFlags::AllowFloat;
                } else if (item.is<bool>()) {
                    valid = field->primitiveTypeFlags & PrimitiveTypeFlags::AllowBool;
                }

                if (!valid) {
                    GlobalLogger.Error(F("Array element has invalid type"), field->name);
                    anyError = true;
                }
            }
        }

        // Validate the JSON array against the given device registry.
        void validateFromRegister(const JsonVariant& jsonArray, const Registry::Item* reg, bool& anyError) {

            if (jsonArray.is<JsonArray>() == false) {
                GlobalLogger.Error(F("Json register field is not a array"));
                anyError = true;
                return; // can't continue here
            }
            const JsonArray& items = jsonArray.as<JsonArray>();
            if (items.size() == 0) {
                GlobalLogger.Error(F("Json register array is empty"));
                anyError = true;
                return; // can't continue here
            }
            uint32_t itemCount = items.size();
            
            for (uint32_t i = 0; i < itemCount; ++i) {
                const JsonVariant& jsonItem = items[i];
                if (jsonItem.is<const char*>()) { continue; } // comment item
                // TODO make this optional so that we can validate disabled items as well
                if (DALHAL::Device::DisabledInJson(jsonItem)) { continue; } // disabled

                bool anyErrorTemp = false;
                validateField(jsonItem, reg->typeName, &JsonSchema::typeField, anyErrorTemp); // this will internally emit errors to log
                if (anyErrorTemp == true) {
                    anyError = true;
                    continue; // skip the current json device
                }
                const char* type_cStr = jsonItem[DALHAL_COMMON_CFG_NAME_TYPE];
                const Registry::Item& regItem = Registry::GetItem(reg, type_cStr);
                if (regItem.typeName == nullptr) {
                    GlobalLogger.Error(F("could not find type:"),type_cStr);
                    anyError = true;
                    continue; // skip the current json device
                }

                if (regItem.def == nullptr) {
                    GlobalLogger.Error(F("FATAL regitem.def == nullptr"));
                    anyError = true;
                    continue; // skip the current json device
                }
                
                if (regItem.def->jsonSchema == nullptr) {
                    GlobalLogger.Error(F("FATAL regItem.def->jsonSchema == nullptr"));

                    anyError = true;
                    continue; // skip the current json device
                }

                validateJsonObject(jsonItem, reg->typeName, regItem.def->jsonSchema, anyError);
            }
        }

    }

}