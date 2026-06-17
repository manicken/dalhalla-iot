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

#include "DALHAL_TX433_Unit.h"

#include <DALHAL/Core/JsonConfig/DALHAL_ArduinoJSON_ext.h>
#include <DALHAL/Support/DALHAL_Logger.h>
#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>

#include <DALHAL/Support/ConvertHelper.h>

#include <DALHAL/Core/Types/DALHAL_Registry.h>

#include "DALHAL_TX433_Unit_TypeLC_JSON_Schema.h"
#include "DALHAL_TX433_Unit_TypeSFC_JSON_Schema.h"
#include "DALHAL_TX433_Unit_TypeAFC_JSON_Schema.h"
#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Base.h>

#include "DALHAL_TX433_UnitTypeRegistry.h"

namespace DALHAL {

    __attribute__((used, externally_visible))
    constexpr TX433_UNIT_RegistryDefine TX433_Unit::LCTypeRegistryDefine = {
        &Create,
        &JsonSchema::TX433_Unit_TypeLC::Root,
        DALHAL_REACTIVE_EVENT_TABLE(TX433_UNIT),
        &TX433_Unit::FunctionTable,
        &JsonSchema::TX433_Unit_TypeLC::Extractors::Apply,
    };
    
    /* override */
    const Registry::DefineBase* TX433_Unit::GetRegistryDefine() {
        // calling it like this is safe as type is allways defined
        // in every instance
        return Registry::GetItem(TX433_UnitTypeRegistry, this->Type).def;
    }
    
    __attribute__((used, externally_visible))
    constexpr TX433_UNIT_RegistryDefine TX433_Unit::SFCTypeRegistryDefine = {
        &Create,
        &JsonSchema::TX433_Unit_TypeSFC::Root,
        DALHAL_REACTIVE_EVENT_TABLE(TX433_UNIT),
        &TX433_Unit::FunctionTable,
        &JsonSchema::TX433_Unit_TypeSFC::Extractors::Apply,
    };

    __attribute__((used, externally_visible))
    constexpr TX433_UNIT_RegistryDefine TX433_Unit::AFCTypeRegistryDefine = {
        &Create,
        &JsonSchema::TX433_Unit_TypeAFC::Root,
        DALHAL_REACTIVE_EVENT_TABLE(TX433_UNIT),
        &TX433_Unit::FunctionTable,
        &JsonSchema::TX433_Unit_TypeAFC::Extractors::Apply,
    };

    constexpr FunctionEntry<FunctionTypes::WriteHALValue> TX433_Unit::writeValueFunctions[] = {
        DALHAL_PRIMARY_FUNCTION_ENTRY(HALValue_primary_write, "write value")
    };

    __attribute__((used, externally_visible))
    constexpr DeviceFunctionTable TX433_Unit::FunctionTable = {
        EmptyFunctionTable<FunctionTypes::Exec>,
        EmptyFunctionTable<FunctionTypes::ReadToHALValue>,
        DALHAL_FUNCTION_TABLE_ENTRY(writeValueFunctions),
        EmptyFunctionTable<FunctionTypes::BracketOpRead>,
        EmptyFunctionTable<FunctionTypes::BracketOpWrite>,
        EmptyFunctionTable<FunctionTypes::ReadString>,
        EmptyFunctionTable<FunctionTypes::WriteString>,
    };

    Device* TX433_Unit::Create(DeviceCreateContext& context) {
        return new TX433_Unit(static_cast<TX433_Unit_CreateFunctionContext&>(context));
    }
    
    TX433_Unit::TX433_Unit(TX433_Unit_CreateFunctionContext& context) : TX433unit_DeviceBase(context.deviceType), pin(context.pin) {
        uid = encodeUID(JsonSchema::CommonBase::uidFieldRequired.ExtractFrom(*(context.jsonObjItem)));
        context.ApplyFunction(context, this);
    }

    HALOperationResult TX433_Unit::HALValue_primary_write(Device* device, const HALValue &val) {
        TX433_Unit& self = static_cast<TX433_Unit&>(*device);
        if (!val.isBoolCompatible()) return HALOperationResult::WriteValueNaN;
        RF433::init(self.pin); // ensure that the correct pin is used and that it's set to a output
        if (self.model == TX433_MODEL::FixedCode) {
            if (self.fixedState == false) {
                RF433::SendTo433_FC(self.staticData, val.toUInt());
            } else {
                RF433::SendTo433_FC(self.staticData);
            }
        }
        else if (self.model == TX433_MODEL::LearningCode) {
            if (self.fixedState == false) {
                RF433::SendTo433_LC(self.staticData, val.toUInt());
            } else {
                RF433::SendTo433_LC(self.staticData);
            }
        } else {
            return HALOperationResult::ExecutionFailed; // this will never happend
        }
#if HAS_REACTIVE_WRITE(TX433_UNIT)
        self.triggerWrite();
#endif
        return HALOperationResult::Success;
    }

    void TX433_Unit::PrintTo(StringBuilderStreamer& sbs) {
        Device::PrintTo(sbs);
        
        sbs.write_json_value_separator();
        sbs.write_jsonMemberStart(F("data"));
        sbs.write_doublequote();
        sbs.write_asHex(staticData);
        sbs.write_doublequote();
    }

}