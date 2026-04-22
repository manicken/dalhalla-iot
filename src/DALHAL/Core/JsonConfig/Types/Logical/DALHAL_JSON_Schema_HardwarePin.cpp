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

#include "DALHAL_JSON_Schema_HardwarePin.h"

#include <stdlib.h>
#include <DALHAL/Support/DALHAL_Logger.h>

#include <DALHAL/Core/Manager/DALHAL_GPIO_Manager.h>
#include <DALHAL/Support/ConvertHelper.h> // for Convert::toBin & Convert::toHex

#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_TypeBase.h>
#include <DALHAL/Core/JsonConfig/Types/Base/DALHAL_JSON_Schema_ValidatorResult.h>

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_TypesRegistry.h>

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_ToJsonStringHelpers.h>

namespace DALHAL {

    namespace JsonSchema {

        constexpr FieldTypeRegistryDefine SchemaHardwarePin::RegistryDefine = {
              &ValidateSchema,
              &ValidateJson,
              &GetValue,
              &SchemaToJson,
              &GetJavaScriptValidator
        };
        
        void SchemaHardwarePin::ValidateSchema(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, bool& anyError) {
            if (SchemaTypeBase::ValidateSchemaNameNotNull(fieldSchema, sourceObjTypeName) == false) {
                anyError = true;
            }
            auto fs = static_cast<const SchemaHardwarePin&>(fieldSchema);
            if (fs.mode == 0) {
                GlobalLogger.Error(F("schema error - SchemaHardwarePin - mode is not set"), sourceObjTypeName);
                anyError = true;
            }
        }

        ValidatorResult SchemaHardwarePin::ValidateJson(const SchemaTypeBase& fieldSchema, const char* sourceObjTypeName, const JsonVariant& jsonObj, bool& anyError) {
            ValidatorResult vRes = SchemaTypeBase::ValidateFieldPresenceAndPolicy(fieldSchema, sourceObjTypeName, jsonObj, anyError);
            if (vRes != ValidatorResult::Success) {
                return vRes; 
            }
            auto value = jsonObj[fieldSchema.name];
            if (value.is<int8_t>() == false) {
                std::string errStr = fieldSchema.name; errStr += " @ ";
                serializeCollapsed(jsonObj, errStr);
                GlobalLogger.Error(F("Harware Pin is not a valid type: "), errStr.c_str());
                anyError = true;
                return ValidatorResult::FieldTypeMismatch;
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
                return ValidatorResult::Success;
            }
            DALHAL_GPIO_MGR_PINFUNC_TYPE fMode = static_cast<const SchemaHardwarePin&>(fieldSchema).mode;
            
            GPIO_manager::CheckPinResult cpRes = GPIO_manager::CheckIfPinAvailableAndIsFree_ThenReserve(pin, fMode);
            if (cpRes != GPIO_manager::CheckPinResult::Success) {
                GPIO_manager::CheckPinResultError errMsg = GPIO_manager::GetCheckPinResultError(cpRes, pin, fMode);
                std::string errStr = errMsg.msg + ", fName=" + fieldSchema.name; errStr += " @ ";
                serializeCollapsed(jsonObj, errStr);
                GlobalLogger.Error(F(errMsg.baseMsg), errStr.c_str());
                anyError = true;
                return ValidatorResult::FieldInvalidValue;
            }
            
            return ValidatorResult::Success;
        }

        HALValue SchemaHardwarePin::GetValue(const SchemaTypeBase& fieldSchema, const JsonVariant& jsonObj) {
            if (jsonObj.containsKey(fieldSchema.name)) {
                return HALValue(jsonObj[fieldSchema.name].as<signed int>());
            } else {
                return HALValue(-1);
            }
        }

        void SchemaHardwarePin::SchemaToJson(const SchemaTypeBase& fieldSchema, std::string& out) {
            SchemaTypeBase::SchemaToJson(fieldSchema, out);
            auto fs = static_cast<const SchemaHardwarePin&>(fieldSchema);

            std::string modeStr = GPIO_manager::describePinFunctions(fs.mode); // this is the most describable version, use this for development test only
            //std::string modeStr = Convert::toHex(fs.mode); // this is the most compact version
            //std::string modeStr = Convert::toBin(fs.mode)
            out += ','; ToJsonString::appendString(out, "mode", modeStr.c_str());
            
            
            if (fieldSchema.type == FieldType::HardwarePin) { 
                out += '}'; // add the object finalizer if this is the actual object
            }
        }

        const char* SchemaHardwarePin::GetJavaScriptValidator() {
            return R"rawliteral(

            )rawliteral";
        }

    }

}