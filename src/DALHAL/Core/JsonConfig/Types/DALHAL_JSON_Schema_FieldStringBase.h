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
#include <ArduinoJson.h>
#include <DALHAL/Support/DALHAL_Logger.h>
#include "DALHAL_JSON_Schema_BaseTypes.h"

namespace DALHAL {

    namespace JsonSchema {

        struct FieldStringBase : FieldBase {

            const char* defaultValue;  // flash string default, or more like what to present at GUI

        protected:
            constexpr FieldStringBase(const char* n, FieldType t, FieldPolicy pol, const char* defVal) 
                : FieldBase(n, t, pol), defaultValue(defVal) {}

            constexpr FieldStringBase(const char* n, FieldType t, FieldPolicy pol, FieldGuiFlags guiFlags, const char* defVal) 
                : FieldBase(n, t, pol, guiFlags), defaultValue(defVal) {}
            
            constexpr FieldStringBase(const char* n, FieldType t, FieldPolicy pol) 
                : FieldBase(n, t, pol), defaultValue(nullptr) {}

            constexpr FieldStringBase(const char* n, FieldType t, FieldPolicy pol, FieldGuiFlags guiFlags) 
                : FieldBase(n, t, pol, guiFlags), defaultValue(nullptr) {}
            
        public:
            constexpr FieldStringBase(const char* n, FieldPolicy pol, const char* defVal) 
                : FieldBase(n, FieldType::StringBase, pol), defaultValue(defVal) {}

            constexpr FieldStringBase(const char* n, FieldPolicy pol, FieldGuiFlags guiFlags, const char* defVal) 
                : FieldBase(n, FieldType::StringBase, pol, guiFlags), defaultValue(defVal) {}

            constexpr FieldStringBase(const char* n, FieldPolicy pol) 
                : FieldBase(n, FieldType::StringBase, pol), defaultValue(nullptr) {}

            constexpr FieldStringBase(const char* n, FieldPolicy pol, FieldGuiFlags guiFlags) 
                : FieldBase(n, FieldType::StringBase, pol, guiFlags), defaultValue(nullptr) {}

	
            inline bool Validate(const JsonVariant& value) {
                if (!value.is<const char*>()) {
                    GlobalLogger.Error(F("Field must be a string:"), this->name);
                    return false;
                }
                ZeroCopyString zcStr = value.as<const char*>(); // wrap in ZeroCopyString for neat functions
                zcStr.Trim();
                size_t strLen = zcStr.Length(); // use of lenght here is fast
                
                if (strLen == 0) {
                    GlobalLogger.Error(F("String is empty @ allowedValues mode: "), this->name);
                    return false;
                }
                return true;
            }
            
            inline const char* GetJavaScriptValidator() {
                return R"rawliteral(
                    function validateString(value) {
                        if (value == undefined) {
                            // emit not a string error here
                            return false;
                        }
                        if (value.length == 0) {
                            // emit string empty error here
                            return false;
                        }
                        // no other validation is needed on basic strings
                        return true;
                    }
                )rawliteral";
            
            }

            
        
        };

    }

}