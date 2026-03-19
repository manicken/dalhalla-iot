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

#include <DALHAL/Core/Device/DALHAL_Device.h>
#include <DALHAL/Core/Reactive/DALHAL_ReactiveEventDescriptor.h>
#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_BaseTypes.h>

#include <ArduinoJson.h> // this dependency can be removed when I begin to use the new config scheme validator

#include <string>

namespace DALHAL {

    struct DeviceCreateContext
    {
        const JsonVariant* jsonObjItem;
        const char* deviceType;
        DeviceCreateContext(): jsonObjItem(nullptr), deviceType("NOT_SET") { }
    };

    namespace Registry {
        //typedef Device* (*HAL_DEVICE_CREATE_FUNC)(const JsonVariant &json, const char* type, void* context);
        typedef Device* (*HAL_DEVICE_CREATE_FUNC)(DeviceCreateContext& context);
        typedef bool (*HAL_DEVICE_VERIFY_JSON_FUNC)(const JsonVariant &jsonObj);

        enum class UseRootUID {
            Mandatory,
            Optional,
            Void
        };

        struct DefineBase {
            HAL_DEVICE_CREATE_FUNC Create_Function;
            HAL_DEVICE_VERIFY_JSON_FUNC Verify_JSON_Function; // soon to be obsolete in favor of jsonSchema
            const JsonSchema::JsonObjectScheme* jsonSchema;
            const EventDescriptor* reactiveTable;
            
            // Verify_JSON_Function variants, soon to be obsolete in favor of JsonSchema
            constexpr DefineBase(
                HAL_DEVICE_CREATE_FUNC Create_Function,
                HAL_DEVICE_VERIFY_JSON_FUNC Verify_JSON_Function
            ) :
                Create_Function(Create_Function),
                Verify_JSON_Function(Verify_JSON_Function),
                jsonSchema(nullptr),
                reactiveTable(nullptr)                
            {}

            constexpr DefineBase(
                HAL_DEVICE_CREATE_FUNC Create_Function,
                HAL_DEVICE_VERIFY_JSON_FUNC Verify_JSON_Function,
                const EventDescriptor* reactiveTable
            ) :
                Create_Function(Create_Function),
                Verify_JSON_Function(Verify_JSON_Function),
                jsonSchema(nullptr),
                reactiveTable(reactiveTable)
            {}
            // JsonSchema variants
            constexpr DefineBase(
                HAL_DEVICE_CREATE_FUNC Create_Function,
                const JsonSchema::JsonObjectScheme* jsonSchema
            ) :
                Create_Function(Create_Function),
                Verify_JSON_Function(nullptr),
                jsonSchema(jsonSchema),
                reactiveTable(nullptr)                
            {}

            constexpr DefineBase(
                HAL_DEVICE_CREATE_FUNC Create_Function,
                const JsonSchema::JsonObjectScheme* jsonSchema,
                const EventDescriptor* reactiveTable
            ) :
                Create_Function(Create_Function),
                Verify_JSON_Function(nullptr),
                jsonSchema(jsonSchema),
                reactiveTable(reactiveTable)
            {}
        };
        
        struct Item {
            const char* typeName;
            const Registry::DefineBase* def;

            constexpr Item(const char* typeName, const Registry::DefineBase* def) : typeName(typeName), def(def) {}
        };

        constexpr Registry::Item TerminatorItem = {nullptr, nullptr};

        const Registry::Item& GetItem(const Registry::Item* reg, const char* type);

        std::string ToString(const Registry::Item* reg);
    }

}