/*
  Dalhalla IoT — JSON-configured HAL/DAL + Script Engine
  HAL = Hardware Abstraction Layer
  DAL = Device Abstraction Layer

  Provides IoT firmware building blocks for home automation and smart sensors.

  Copyright (C) 2026 Jannik Svensson

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#pragma once


#include <string>
#include <DALHAL/Core/Types/DALHAL_Registry.h>


namespace DALHAL {

  class TX433_Unit; // forward declaration

  struct TX433_UNIT_RegistryDefine : public Registry::DefineBase {
    using ApplyFn = void (*)(DALHAL::DeviceCreateContext&, DALHAL::TX433_Unit*);
        ApplyFn Apply;

        constexpr TX433_UNIT_RegistryDefine(
            Registry::HAL_DEVICE_CREATE_FUNC Create_Function, 
            const JsonSchema::JsonObjectSchema* jsonSchema,
            ApplyFn Apply
        ) : 
            Registry::DefineBase(Create_Function, jsonSchema),
            Apply(Apply)
        {}

        constexpr TX433_UNIT_RegistryDefine(
            Registry::HAL_DEVICE_CREATE_FUNC Create_Function, 
            const JsonSchema::JsonObjectSchema* jsonSchema,
            const EventDescriptor* reactiveTable,
            ApplyFn Apply
        ) : 
            Registry::DefineBase(Create_Function, jsonSchema, reactiveTable),
            Apply(Apply)
        {}

    };
    
    extern const Registry::Item TX433_UnitTypeRegistry[];

}