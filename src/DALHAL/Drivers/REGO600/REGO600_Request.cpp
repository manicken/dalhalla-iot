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

#include "REGO600_Request.h"
#include <DALHAL/Core/Types/DALHAL_Value.h>
#include "REGO600_ErrorReport.h"

namespace Drivers {
    namespace REGO600 {

        bool Request::unpack_nibbles(const uint8_t* src, uint8_t* dst, size_t dstLen) {
            for (size_t i = 0; i < dstLen; ++i)
            {
                uint8_t high = src[i*2], low = src[i*2+1];
                if (high > 0xF || low > 0xF) {
                    // corrupted data
                    return false;
                }
                dst[i] = (high << 4) + low;
            }
            return true;
        }

        // Constructor for linked values here the type is allways Value
        Request::Request(const OpCodeInfo& _info, const RegoLookupEntry& _def, DALHAL::HALValue& externalValue) 
            : info(_info), 
            def(_def), ownedValue(false)
        {
            response.value = &externalValue;
        }

        Request::Request(const OpCodeInfo& _info, const RegoLookupEntry& _def, char* externalValue) 
            : info(_info), 
            def(_def), ownedValue(false)
        {
            response.text = externalValue;
        }

        Request::Request(const OpCodeInfo& _info, const RegoLookupEntry& _def, uint8_t* externalValue) 
            : info(_info), 
            def(_def), ownedValue(false)
        {
            response.rawBytes = externalValue;
        }

        Request::Request(const OpCodeInfo& _info, const RegoLookupEntry& _def) 
            : info(_info), 
            def(_def), ownedValue(false)
        {
            if (info.responseType == ResponseType::Text) {
                response.text = new char[21](); // 20 characters + null terminator
                ownedValue = true;
            }
            else if (info.responseType == ResponseType::Value) {
                response.value = new DALHAL::HALValue();
                ownedValue = true;
            }
            else if (info.responseType == ResponseType::RawBytes) {
                response.rawBytes = new uint8_t[20]();
                ownedValue = true;
            }
            else if (info.responseType == ResponseType::ErrorLogItem) {
                response.text = new char[20](); // 3 digit error code + space + 6 char date + space + 8 char time + null terminator
                ownedValue = true;
            } else {
                response.value = nullptr;
            }
        }

        Request::ValidateSetResult Request::ValidateAndSetFromBuffer(uint8_t* buff) {
            if (info.responseType == ResponseType::Value && response.value) {
                // 1. Extract the 16-bit raw value (7-bit packing)
                uint16_t rawValue = (buff[1] << 14) + (buff[2] << 7) + buff[3];
                
                // 2. Perform Type-Aware Validation
                switch (def.valueType) {
                    case ValueType::Signed: {
                        int16_t sVal = static_cast<int16_t>(rawValue);
                        if (sVal >= def.minVal.s16 && sVal <= def.maxVal.s16) {
                            response.value->set((int32_t)(sVal*def.multiplier));
                        } else {
                            ErrorReport::DebugMessage(String(F("skipped Signed value because out of range")).c_str());
                            return Request::ValidateSetResult::Retry;
                        }
                        break;
                    }
                    case ValueType::Bool:
                        if (rawValue == 0 || rawValue == 1) {
                            response.value->set((uint32_t)rawValue);
                        } else {
                            ErrorReport::DebugMessage(String(F("skipped Bool value because out of range")).c_str());
                            return Request::ValidateSetResult::Retry;
                        }
                        break;
                    case ValueType::Unsigned:
                        if (rawValue >= def.minVal.u16 && rawValue <= def.maxVal.u16) {
                            response.value->set((uint32_t)(rawValue*def.multiplier));
                        } else {
                            ErrorReport::DebugMessage(String(F("skipped Unsigned value because out of range")).c_str());
                            return Request::ValidateSetResult::Retry;
                        }
                        break;
                    case ValueType::Float: { 
                        int16_t sVal = static_cast<int16_t>(rawValue);
                        if (sVal >= def.minVal.s16 && sVal <= def.maxVal.s16) {
                            response.value->set(sVal*def.multiplier);
                        } else {
                            ErrorReport::DebugMessage(String(F("skipped Float value because out of range")).c_str());
                            return Request::ValidateSetResult::Retry;
                        }
                        break;
                    }
                    default: 
                        break; // to avoid making infinite request loop on unset type entities
                }

            } else if (info.responseType == ResponseType::Text) {
                uint8_t tmp[20];
                
                if (!unpack_nibbles(&buff[1], tmp, 20)) {
                    ErrorReport::DebugMessage(String(F("LCD data corruption detected - discarding frame - ")).c_str());
                    return Request::ValidateSetResult::Retry;
                }

                memcpy(response.text, tmp, 20);
                response.text[20] = '\0';
            } else if (info.responseType == ResponseType::ErrorLogItem) {
                
                uint32_t code = buff[1]*16 + buff[2];
                response.text[0] = (code / 100)+0x30;
                code %= 100;
                response.text[1] = (code / 10)+0x30;
                code %= 10;
                response.text[2] = (code)+0x30;
                response.text[3] = 0x20; // space
                for (int bi=3,ti=4;ti<20;bi+=2,ti++) {
                    response.text[ti] = buff[bi]*16 + buff[bi+1];
                }
                
            }/* else if (info.type == RequestType::RawBytes) {
                uint8_t tmp[20];
                
                if (!unpack_nibbles(&buff[1], tmp, 20)) {
                    ErrorReport::DebugMessage(String(F("RAW ram dump data corruption detected - discarding frame - ")).c_str());
                    return Request::ValidateSetResult::Retry;
                }

                memcpy(response.rawBytes, tmp, 20);
            } */else if (info.responseType == ResponseType::WriteConfirm) {
                return Request::ValidateSetResult::Success;
            } else if (info.responseType == ResponseType::NotSet) {
                ErrorReport::DebugMessage(String(F("responseType not set - discarding frame - ")).c_str());
                return Request::ValidateSetResult::UnsetType;   
            } else {
                ErrorReport::DebugMessage(String(F("unknown responseType - discarding frame - ")).c_str());
                return Request::ValidateSetResult::UnknownType;   
                // there are currently no more types right now
            }
            return Request::ValidateSetResult::Success;
        }

        REGO600::Request::~Request() {
            // delete owning
            if (ownedValue) {
                if (info.responseType == ResponseType::Text) {
                    delete[] response.text;
                }
                else if (info.responseType == ResponseType::Value) {
                    delete response.value;
                }
                else if (info.responseType == ResponseType::RawBytes) {
                    delete[] response.rawBytes;
                }
                else if (info.responseType == ResponseType::ErrorLogItem) {
                    delete[] response.text;
                }
            }
        }

        std::string REGO600::Request::ToString() {
            std::string str = "";
            str += "info.op: " + std::to_string((int)info.opcode);
            str += ", info.size: " + std::to_string((int)info.size);
            str += ", def.name: " + std::string(def.name);
            str += ", def.address: " + std::to_string(def.address);
            if (def.valueType == ValueType::Signed || def.valueType == ValueType::Float) {
                str += ", def.minVal.s16: " + std::to_string(def.minVal.s16);
                str += ", def.maxVal.s16: " + std::to_string(def.maxVal.s16);
            } else {
                str += ", def.minVal.u16: " + std::to_string(def.minVal.u16);
                str += ", def.maxVal.u16: " + std::to_string(def.maxVal.u16);
            }

            return str;
        }

    }
}