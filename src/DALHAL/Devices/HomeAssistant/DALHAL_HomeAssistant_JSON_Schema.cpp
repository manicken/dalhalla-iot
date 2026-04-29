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

#include "DALHAL_HomeAssistant_JSON_Schema.h"

#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_TypeBase.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/Groups/DALHAL_JSON_Schema_FieldsGroup.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/Groups/DALHAL_JSON_Schema_AllOfFieldsGroup.h>
#include <DALHAL/Core/JsonConfig/Types/Logical/Groups/DALHAL_JSON_Schema_OneOfFieldsGroup.h>
#include <DALHAL/Core/JsonConfig/Types/Structures/DALHAL_JSON_Schema_ArrayOfObjects.h>
#include <DALHAL/Core/JsonConfig/Types/Structures/DALHAL_JSON_Schema_Object.h>
#include <DALHAL/Core/JsonConfig/Types/Primitives/DALHAL_JSON_Schema_String.h>
#include <DALHAL/Core/JsonConfig/Types/Primitives/DALHAL_JSON_Schema_UInt.h>
#include <DALHAL/Core/JsonConfig/Types/Structures/DALHAL_JSON_Schema_ArrayOfRegistryItems.h>
#include <DALHAL/Core/JsonConfig/Types/Root/DALHAL_JSON_Schema_JsonObjectSchema.h>

#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Base.h>

#include <DALHAL/Devices/HomeAssistant/DALHAL_HA_DeviceTypeReg.h>
#include <DALHAL/Devices/HomeAssistant/DALHAL_HA_CreateFunctionContext.h>

#include "DALHAL_HomeAssistant.h"
#include <DALHAL/Devices/HomeAssistant/Core/DALHAL_HA_DeviceDiscovery.h>
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

namespace DALHAL {

    namespace JsonSchema {

        namespace HomeAssistant {

            constexpr SchemaString deviceIdField = {"deviceId", FieldPolicy::Required};
            constexpr SchemaString hostField = {"host", FieldPolicy::Required};
            constexpr SchemaUInt   portField = {"port", FieldPolicy::Required, (unsigned int)1, (unsigned int)65535, (unsigned int)1883};
            constexpr SchemaString userField = {"user", FieldPolicy::AllOfFieldsGroup};
            constexpr SchemaString passField = {"pass", FieldPolicy::AllOfFieldsGroup};

            constexpr const SchemaTypeBase* credentialsFields[] = {&userField, &passField, nullptr};
            constexpr SchemaAllOfFieldsGroup credentialsGroup = {"credentials", FieldPolicy::Optional, credentialsFields};

            constexpr SchemaString groupNameField = {"name", FieldPolicy::Required};
            constexpr SchemaString groupUIDField = {"uid", FieldPolicy::Required};

            constexpr const SchemaTypeBase* globalGroupFields[] = {&groupUIDField, &groupNameField, nullptr};
            constexpr JsonObjectSchema globalGroupSchema = {
                "GlobalGroup",
                globalGroupFields,
                nullptr, // no modes
                nullptr,  // no constraints
                EmptyPolicy::Warn,
                UnknownFieldPolicy::Warn,
            };
            constexpr SchemaObject globalGroupField = {"group", FieldPolicy::ModeDefine, &globalGroupSchema};

            constexpr SchemaArrayOfRegistryItems itemsField = {"items", FieldPolicy::ModeDefine, DALHAL::HA_DeviceRegistry, "ROOT.HOMEASSISTANT"};

            constexpr const SchemaTypeBase* individualGroupFields[] = {&CommonBase::uidFieldRequired, &groupNameField, &itemsField, nullptr};
            constexpr JsonObjectSchema individualGroupSchema = {
                "IndividualGroup",
                individualGroupFields,
                nullptr, // no modes
                nullptr,  // no constraints
                EmptyPolicy::Warn,
                UnknownFieldPolicy::Warn,
            };
            constexpr SchemaArrayOfObjects individualGroupsField = {"groups", FieldPolicy::ModeDefine, &individualGroupSchema};

            constexpr const SchemaTypeBase* fields[] = {
                &CommonBase::disabled_type_uidreq_note_group, // DALHAL_CommonSchemas_Base
                &deviceIdField,
                &hostField,
                &portField,
                &credentialsGroup,
                &globalGroupField,
                &itemsField,
                &individualGroupsField,
                nullptr,
            };

            constexpr ModeConjunctionDefine globalGroupModeConjunctions[] = {
                { &globalGroupField, true },      // group must exist for this mode
                { &itemsField, true },            // items must exist
                { &individualGroupsField, false },// groups must NOT exist
                { nullptr, false}
            };

            constexpr ModeConjunctionDefine individualGroupModeConjunctions[] = {
                { &globalGroupField, false },      // group must NOT exist for this mode
                { &itemsField, false },            // items must NOT exist
                { &individualGroupsField, true },  // groups must exist
                { nullptr, false}
            };

            constexpr ModeSelector modes[] = {
                {"global group mode", globalGroupModeConjunctions, Extractors::ExtractGlobalGroupMode},
                {"individual groups mode", individualGroupModeConjunctions, Extractors::ExtractIndividualGroupMode},
                {nullptr, nullptr, nullptr}
            };

            constexpr JsonObjectSchema Root = {
                "HomeAssistant",
                fields,
                modes,
                nullptr,  // no constraints
                EmptyPolicy::Warn,
                UnknownFieldPolicy::Warn,
            };

            void Extractors::Apply(const DALHAL::DeviceCreateContext& context, DALHAL::HomeAssistant* out) {
                out->uid = encodeUID(JsonSchema::CommonBase::uidFieldRequired.ExtractFrom(*(context.jsonObjItem)));
                out->deviceID = std::string(JsonSchema::HomeAssistant::deviceIdField.ExtractFrom(*(context.jsonObjItem)));
                out->host = std::string(JsonSchema::HomeAssistant::hostField.ExtractFrom(*(context.jsonObjItem)));
                out->port = JsonSchema::HomeAssistant::portField.ExtractFrom(*(context.jsonObjItem));
                // note. can only be called after getting host and port
                out->ConfigureMqttClient();
                out->username = std::string(JsonSchema::HomeAssistant::userField.ExtractFrom(*(context.jsonObjItem)));
                out->password = std::string(JsonSchema::HomeAssistant::passField.ExtractFrom(*(context.jsonObjItem)));
                // note. can only be called after getting username and password
                out->Connect();

                JsonSchema::ModeSelector::Apply(JsonSchema::HomeAssistant::Root.modes, context, out);
            }

            void Extractors::CreateDevicesFromItems(const JsonArray& items, DALHAL::HA_CreateFunctionContext& createFuncContext, DALHAL::Device** devices, int& index) {
                int arrayCount = items.size();
                for (int i=0;i<arrayCount;i++) {
                    const JsonVariant& item = items[i];
                    if (Device::DisabledOrCommentItem(item)) { continue; }

                    const char* type_cStr = JsonSchema::CommonBase::typeField.ExtractFrom(item);
                    
                    const Registry::Item& regItem = Registry::GetItem(HA_DeviceRegistry, type_cStr);
                    createFuncContext.jsonObjItem = &item;
                    createFuncContext.deviceType = regItem.typeName; // type_cStr cannot be used here as that is a json string
                    devices[index++] = regItem.def->Create_Function(createFuncContext);
                }
            }

            void Extractors::ExtractGlobalGroupMode(const DALHAL::DeviceCreateContext& context, void* out) {
                auto* self = static_cast<DALHAL::HomeAssistant*>(out);

                const JsonVariant& jsonObj = *context.jsonObjItem;
                const JsonArray& jsonArrayItems = JsonSchema::HomeAssistant::itemsField.GetValidatedJsonArray(jsonObj);
                int arrayCount = jsonArrayItems.size();

                int validItemCount = 0;
                // first pass count and check valid items
                for (int i=0;i<arrayCount;i++) {
                    const JsonVariant& item = jsonArrayItems[i];
                    if (Device::DisabledOrCommentItem(item)) { continue; }
                    validItemCount++;
                }
                self->deviceCount = validItemCount;
                self->devices = new Device*[validItemCount](); // create array and initialize all to nullptr
                int index = 0;
                // second pass create devices
                const JsonVariant& groupObj = JsonSchema::SchemaObject::GetValidatedJsonObject(JsonSchema::HomeAssistant::globalGroupField, jsonObj);
                
                HA_CreateFunctionContext createFuncContext(self->mqttClient);
                createFuncContext.jsonGlobal = &groupObj;
                createFuncContext.deviceId_cStr = JsonSchema::HomeAssistant::deviceIdField.ExtractFrom(*(context.jsonObjItem));
                CreateDevicesFromItems(jsonArrayItems, createFuncContext, self->devices, index);
                
                
                /*for (int i=0;i<arrayCount;i++) {
                    const JsonVariant& item = jsonArrayItems[i];
                    if (Device::DisabledOrCommentItem(item)) { continue; }

                    const char* type_cStr = JsonSchema::CommonBase::typeField, item).asConstChar();
                    
                    const Registry::Item& regItem = Registry::GetItem(HA_DeviceRegistry, type_cStr);
                    createFuncContext.jsonObjItem = &item;
                    createFuncContext.deviceType = regItem.typeName; // type_cStr cannot be used here as that is a json string
                    self->devices[index++] = regItem.def->Create_Function(createFuncContext);
                }*/



            }

            void Extractors::ExtractIndividualGroupMode(const DALHAL::DeviceCreateContext& context, void* out) {
                auto* self = static_cast<DALHAL::HomeAssistant*>(out);

                const JsonVariant& jsonObj = *context.jsonObjItem;
                const JsonArray& jsonArrayGroups = JsonSchema::HomeAssistant::individualGroupsField.GetValidatedJsonArray(jsonObj);
                int jsonArrayGroupsCount = jsonArrayGroups.size();
                
                int activeItemCount = 0;
                // first pass count enabled/"non comment" items
                for (int i=0;i<jsonArrayGroupsCount;i++) {
                    const JsonVariant& jsonObjGrpItem = jsonArrayGroups[i];
                    const JsonArray& jsonArrayItems = JsonSchema::HomeAssistant::itemsField.GetValidatedJsonArray(jsonObjGrpItem);
                    int jsonArrayItemsCount = jsonArrayItems.size();
                    for (int j=0;j<jsonArrayItemsCount;j++) {
                        const JsonVariant& item = jsonArrayItems[j];
                        if (Device::DisabledOrCommentItem(item)) { continue; }
                        activeItemCount++;
                    }
                }

                // second pass create actual enabled/"non comment" items
                self->deviceCount = activeItemCount;
                self->devices = new Device*[activeItemCount]();

                int newItemIndex = 0;
                HA_CreateFunctionContext createFuncContext(self->mqttClient);
                //createFuncContext.jsonObjRoot = &jsonObj;
                for (int i=0;i<jsonArrayGroupsCount;i++) {
                    const JsonVariant& jsonObjGrpItem = jsonArrayGroups[i];
                    const JsonArray& jsonArrayItems = JsonSchema::HomeAssistant::itemsField.GetValidatedJsonArray(jsonObjGrpItem);

                    createFuncContext.jsonGlobal = &jsonObjGrpItem;
                    
                    CreateDevicesFromItems(jsonArrayItems, createFuncContext, self->devices, newItemIndex);


                    /*for (int j=0;j<jsonArrayItemsCount;j++) {
                        const JsonVariant& item = jsonArrayItems[j];
                        if (Device::DisabledOrCommentItem(item)) { continue; }

                        const char* type_cStr = JsonSchema::CommonBase::typeField, item).asConstChar();
                
                        const Registry::Item& regItem = Registry::GetItem(HA_DeviceRegistry, type_cStr);
                        createFuncContext.jsonObjItem = &item;
                        createFuncContext.deviceType = regItem.typeName; // type_cStr cannot be used here as that is a json string
                        self->devices[newItemIndex++] = regItem.def->Create_Function(createFuncContext);
                    }*/

                    
                }
            }

        }

    }

}