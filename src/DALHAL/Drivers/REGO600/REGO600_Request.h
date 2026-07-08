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

#include "REGO600_RegoLookupEntry.h"
#include "REGO600_OpCodeInfo.h"
#include <DALHAL/Core/Types/DALHAL_Value.h>

#include <string.h>
#include <stdint.h>

namespace Drivers {
    namespace REGO600 {

        enum class RequestMode {
            RefreshLoop,
            Lcd,
            FrontPanelLeds,
            OneTime
        };

        using RequestCallback = void (*)(void* cb_ctx, void* dataCtx, RequestMode);


        struct Request {

            static bool unpack_nibbles(const uint8_t* src, uint8_t* dst, size_t dstLen);
            
            const OpCodeInfo& info;
            const RegoLookupEntry& def;

            union Response { // type name this so that it can be passed
                DALHAL::HALValue* value;
                char* text;
                uint8_t* rawBytes;
            } response;

            bool ownedValue = false;

            Request() = delete;
            Request(const OpCodeInfo& _info, const RegoLookupEntry& _def);
            /** externalValue is a non-owning pointer to external data. 
             * Do not delete; must remain valid during Request's lifetime.
             */
            Request(const OpCodeInfo& _info, const RegoLookupEntry& _def, DALHAL::HALValue& externalValue);
            /** externalValue is a non-owning pointer to external data. 
             * Do not delete; must remain valid during Request's lifetime.
             */
            Request(const OpCodeInfo& _info, const RegoLookupEntry& _def, char* externalValue);
            /** externalValue is a non-owning pointer to external data. 
             * Do not delete; must remain valid during Request's lifetime.
             */
            Request(const OpCodeInfo& _info, const RegoLookupEntry& _def, uint8_t* externalValue);

            enum class ValidateSetResult {
                UnsetType,
                UnknownType,
                Retry,
                Success
            };
            ValidateSetResult ValidateAndSetFromBuffer(uint8_t* buff);

            std::string ToString();

            ~Request();

            // prevent accidental copies/moves
            Request(const Request&) = delete;
            Request& operator=(const Request&) = delete;
            Request(Request&&) = delete;
            Request& operator=(Request&&) = delete;

            
        };

        

    }
}