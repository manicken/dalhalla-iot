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

#include "DALHAL_Display_SSD1306_Element.h"

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>

#include <DALHAL/Core/JsonConfig/DALHAL_ArduinoJSON_ext.h>
#include <DALHAL/Support/DALHAL_Logger.h>

#include <DALHAL/Core/Types/DALHAL_Registry.h>

#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Base.h>
#include <DALHAL/Core/JsonConfig/CommonSchemas/DALHAL_CommonSchemas_Consumer.h>

#include "DALHAL_Display_SSD1306_JSON_Schema.h"

namespace DALHAL {
    
    Display_SSD1306_Element::Display_SSD1306_Element(DeviceCreateContext& context) : Device(context.deviceType) {
        uid = encodeUID(JsonSchema::GetValue(JsonSchema::CommonBase::uidFieldRequired, context).asConstChar());
        xPos = JsonSchema::GetValue(JsonSchema::Display_SSD1306::xField, context);
        yPos = JsonSchema::GetValue(JsonSchema::Display_SSD1306::yField, context);
        label = std::string(JsonSchema::GetValue(JsonSchema::Display_SSD1306::labelField, context).asConstChar());

        const char* sourceStr = JsonSchema::GetValue(JsonSchema::CommonConsumer::sourceField, context).asConstChar();
        if (sourceStr != nullptr) {
            cdaSource = new CachedDeviceAccess();
            if (cdaSource->Set(sourceStr) == false) {
                delete cdaSource;
                cdaSource = nullptr;
            }
        }
        else
            cdaSource = nullptr;
    }

    String Display_SSD1306_Element::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += this->Type;
        return ret;
    }

    HALOperationResult Display_SSD1306_Element::write(const HALValue& val) {
        if (val.getType() == HALValue::Type::TEST) return HALOperationResult::Success; // test write to check feature
        if (val.isNaN()) return HALOperationResult::WriteValueNaN;
        this->val = val;
        return HALOperationResult::Success;
    }

}