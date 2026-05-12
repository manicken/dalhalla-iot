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

#include <stdlib.h>
#include <WString.h>

namespace DALHAL {

    namespace JsonSchema {

        /** used in situations when Validate functions need to pass special conditions */
        enum class ValidatorResult {
            Success,
            SchemaTypeNotFound,
            FieldTypeMismatch,
            FieldEmpty,
            RequiredFieldMissing,
            FieldMissing,  // not a error and can be used when the type is inherited
            FieldInvalidValue
        };
        const __FlashStringHelper* ValidatorResultToString(ValidatorResult res);

    }

}