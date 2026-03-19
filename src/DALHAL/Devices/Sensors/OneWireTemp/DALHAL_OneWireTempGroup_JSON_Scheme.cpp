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

#include "DALHAL_OneWireTempGroup_JSON_Scheme.h"

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_Types.h>
#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_BaseTypes.h>
#include <DALHAL/Core/Manager/DALHAL_GPIO_Manager.h>
#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>

#include <DALHAL/Core/JsonConfig/CommonSchemes/DALHAL_CommonSchemes_Base.h>
#include <DALHAL/Core/JsonConfig/CommonSchemes/DALHAL_CommonSchemes_Pins.h>
#include <DALHAL/Core/JsonConfig/CommonSchemes/DALHAL_CommonSchemes_Time.h>

#include "DALHAL_OneWireTempBus_JSON_Scheme.h"

namespace DALHAL {

    namespace JsonSchema {

        constexpr FieldArray itemsField = {"items", FieldFlag::Required, &OneWireTempBus, EmptyPolicy::Error};

        constexpr const FieldBase* fields[] = {
            &noteField,         // DALHAL_CommonSchemes_Base, GUI optional note field
            &typeField,         // DALHAL_CommonSchemes_Base
            &uidFieldRequired,  // DALHAL_CommonSchemes_Base
            &refreshTimeGroupFieldsRequired, // required for now as this device need to run refresh in background
                                             // if it should not depend on automatic refresh, it do need a cmd that start a conversion
                                             // that then emit a reactive event when the convertion is done
            &itemsField,
        };

        constexpr JsonObjectScheme OneWireTempGroup = { // allways at root
            "OneWireTempGroupAtRoot",
            fields,
            nullptr, // no modes
            nullptr,  // no constraints
            EmptyPolicy::Warn,
            UnknownFieldPolicy::Warn,
        };

    }

}