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

#pragma once

#include "DALHAL_DeviceCreateContext.h"

#include <DALHAL/API/DALHAL_CommandExecutor.h>

//#include <DALHAL/Core/Types/DALHAL_Device.h>
//#include <DALHAL/Core/Types/DALHAL_DeviceFunctionTable.h>
#include <DALHAL/Core/Reactive/DALHAL_ReactiveEventDescriptor.h>
#include <DALHAL/Core/JsonConfig/Types/Root/DALHAL_JSON_Schema_JsonObjectSchema.h>

#include <ArduinoJson.h> // this dependency can be removed when I begin to use the new config schema validator

#include <string>

namespace DALHAL {

    // forward declarations
    class Device;
    struct DeviceFunctionTable;

    namespace Registry {
        
        typedef Device* (*HAL_DEVICE_CREATE_FUNC)(DeviceCreateContext& context);
  
        enum class UseRootUID {
            Mandatory,
            Optional,
            Void
        };

        struct DefineBase {
            HAL_DEVICE_CREATE_FUNC Create_Function;
            const JsonSchema::JsonObjectSchema* jsonSchema;
            const EventDescriptor* reactiveTable;
            const DeviceFunctionTable* functionTable;
            
            constexpr DefineBase(
                HAL_DEVICE_CREATE_FUNC Create_Function,
                const JsonSchema::JsonObjectSchema* jsonSchema
            ) :
                Create_Function(Create_Function),
                jsonSchema(jsonSchema),
                reactiveTable(nullptr),
                functionTable(nullptr)              
            {}

            constexpr DefineBase(
                HAL_DEVICE_CREATE_FUNC Create_Function,
                const JsonSchema::JsonObjectSchema* jsonSchema,
                const EventDescriptor* reactiveTable
            ) :
                Create_Function(Create_Function),
                jsonSchema(jsonSchema),
                reactiveTable(reactiveTable),
                functionTable(nullptr)
            {}

            constexpr DefineBase(
                HAL_DEVICE_CREATE_FUNC Create_Function,
                const JsonSchema::JsonObjectSchema* jsonSchema,
                const DeviceFunctionTable* functionTable
            ) :
                Create_Function(Create_Function),
                jsonSchema(jsonSchema),
                reactiveTable(nullptr),
                functionTable(functionTable)              
            {}

            constexpr DefineBase(
                HAL_DEVICE_CREATE_FUNC Create_Function,
                const JsonSchema::JsonObjectSchema* jsonSchema,
                const EventDescriptor* reactiveTable,
                const DeviceFunctionTable* functionTable
            ) :
                Create_Function(Create_Function),
                jsonSchema(jsonSchema),
                reactiveTable(reactiveTable),
                functionTable(functionTable)
            {}
        };
        
        struct Item {
            const char* typeName;
            const Registry::DefineBase* def;

            constexpr Item(const char* typeName, const Registry::DefineBase* def) : typeName(typeName), def(def) {}
        };

        constexpr Registry::Item TerminatorItem = {nullptr, nullptr};

        const Registry::Item& GetItem(const Registry::Item* reg, const char* type);

        void ToString(const Registry::Item* reg, CommandCallback cb);
    }

}