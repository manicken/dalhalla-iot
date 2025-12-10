/*
  Dalhalla IoT — JSON-configured HAL/DAL + Script Engine
  HAL = Hardware Abstraction Layer
  DAL = Device Abstraction Layer

  Provides IoT firmware building blocks for home automation and smart sensors.

  Copyright (C) 2025 Jannik Svensson

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

#include "HAL_JSON_Device.h"

//#define PRINT_FAIL_OPERATION_DEBUG_MESSAGE

#ifdef PRINT_FAIL_OPERATION_DEBUG_MESSAGE
#define PrintReadFailOperation(KIND) Serial.printf("rd(" KIND ") @ type:%s",type); 
#define PrintWriteFailOperation(KIND) Serial.printf("wr(" KIND ") @ type:%s",type);
#define PrintExecFailOperation(KIND) Serial.printf("exec(" KIND ") @ type:%s",type);
#else
#define PrintReadFailOperation(KIND)
#define PrintWriteFailOperation(KIND)
#define PrintExecFailOperation(KIND)
#endif

namespace HAL_JSON {

    const char* DeviceFindResultToString(DeviceFindResult res) {
        switch (res)
        {
            case DeviceFindResult::DeviceNotFound: return "DeviceNotFound";
            case DeviceFindResult::EmptyUIDPath: return "EmptyUIDPath";
            case DeviceFindResult::InvalidUID: return "InvalidUID";
            case DeviceFindResult::PathTooDeep: return "PathTooDeep";
            case DeviceFindResult::SubDevicesNotSupported: return "SubDevicesNotSupported";
            case DeviceFindResult::SubDeviceListEmpty: return "SubDeviceListEmpty";
            case DeviceFindResult::Success: return "Success";
            default: return "Unknown";
        }
    }

    const char* Device::GetType() { return type; } 

    Device::Device(const char* type) : type(type) { }

    Device::~Device() {}

    void Device::loop() {}
    void Device::begin() {}

    

    DeviceFindResult Device::findDevice(UIDPath& path, Device*& outDevice) { return DeviceFindResult::SubDevicesNotSupported; }

    String Device::ToString() { return ""; }

    HALOperationResult Device::read(HALValue& val) { 
        PrintReadFailOperation("HALValue&");
        return HALOperationResult::UnsupportedOperation;
    }
    HALOperationResult Device::write(const HALValue& val) { 
        PrintWriteFailOperation("HALValue&"); 
        return HALOperationResult::UnsupportedOperation;
    };
    HALOperationResult Device::read(const HALValue& bracketSubscriptVal, HALValue& val) { 
        PrintReadFailOperation("HALValue&, HALValue&"); 
        return HALOperationResult::UnsupportedOperation;
    }
    HALOperationResult Device::write(const HALValue& bracketSubscriptVal, const HALValue& val) { 
        PrintWriteFailOperation("HALValue&, HALValue&"); 
        return HALOperationResult::UnsupportedOperation;
    }
    HALOperationResult Device::read(const HALReadStringRequestValue& val) { 
        PrintReadFailOperation("HALReadStringRequestValue&"); 
        return HALOperationResult::UnsupportedOperation;
    }
    HALOperationResult Device::write(const HALWriteStringRequestValue& val) { 
        PrintWriteFailOperation("HALWriteStringRequestValue&"); 
        return HALOperationResult::UnsupportedOperation; 
    }
    HALOperationResult Device::read(const HALReadValueByCmd& val) { 
        PrintReadFailOperation("HALReadValueByCmd&"); 
        return HALOperationResult::UnsupportedOperation; 
    }
    HALOperationResult Device::write(const HALWriteValueByCmd& val) { 
        PrintWriteFailOperation("HALWriteValueByCmd&"); 
        return HALOperationResult::UnsupportedOperation; 
    }
    HALOperationResult Device::exec() { 
        PrintExecFailOperation(""); 
        return HALOperationResult::UnsupportedOperation; 
    }
    HALOperationResult Device::exec(const ZeroCopyString& zcStr) { 
        PrintExecFailOperation("ZeroCopyString&"); 
        return HALOperationResult::UnsupportedOperation; 
    }

    Device::ReadToHALValue_FuncType Device::GetReadToHALValue_Function(ZeroCopyString& zcFuncName) { return nullptr; }
    Device::WriteHALValue_FuncType Device::GetWriteFromHALValue_Function(ZeroCopyString& zcFuncName) { return nullptr; }
    Device::Exec_FuncType Device::GetExec_Function(ZeroCopyString& zcFuncName) { return nullptr; }

    Device::BracketOpRead_FuncType Device::GetBracketOpRead_Function(ZeroCopyString& zcFuncName) { return nullptr; }
    Device::BracketOpWrite_FuncType Device::GetBracketOpWrite_Function(ZeroCopyString& zcFuncName) { return nullptr; }

    Device::EventCheck_FuncType Device::Get_EventCheck_Function(ZeroCopyString& zcFuncName) { return nullptr; }

    
    
    HALValue* Device::GetValueDirectAccessPtr() { return nullptr; }

    bool Device::DisabledInJson(const JsonVariant& jsonObj) {
        if (jsonObj.containsKey(HAL_JSON_KEYNAME_DISABLED) == false) return false;
        if (jsonObj[HAL_JSON_KEYNAME_DISABLED].is<bool>() == false) return false;
        return jsonObj[HAL_JSON_KEYNAME_DISABLED].as<bool>(); 
    }

    DeviceFindResult Device::findInArray(Device** devices, int deviceCount, UIDPath& path, Device* currentDevice, Device*& outDevice) {
        //outDevice = nullptr; // allways set
        
        if (devices == nullptr || *devices == nullptr || deviceCount == 0) {
            GlobalLogger.Error(F("Could not continue search at device type: "), currentDevice?currentDevice->type:"root");
            return DeviceFindResult::SubDeviceListEmpty;
        }
        if (path.empty()) {
            return DeviceFindResult::EmptyUIDPath;
        }
        
        // Determine which UID to compare/consume at this level
        HAL_UID currUID;
        if (currentDevice && currentDevice->uid.IsSet()) {
            currUID = path.getNextUID();   // advance for subdevice
        } else {
            currUID = path.getCurrentUID(); // root level or placeholder
        }

        if (currUID.Invalid()) {
            return DeviceFindResult::InvalidUID;
        }

        for (int i = 0; i < deviceCount; i++) {
            Device* dev = devices[i];
            if (!dev) continue; // failsafe

            //----------------------------------------------------------------------
            // 1) Direct match on UID
            //----------------------------------------------------------------------
            if (dev->uid == currUID) { // only for absolute paths

                if (path.hasMore() == false) {
                    // exact/current level match, path ends here
                    outDevice = dev;
                    return DeviceFindResult::Success;
                }
                // recurse find
                return dev->findDevice(path, outDevice);
                
            }
            //----------------------------------------------------------------------
            // 2) Placeholder device (uid.NotSet())
            //
            // Can recurse if ANY of these are true:
            //   A) It is root device             → currentDevice == null
            //   B) More segments remain          → !path.isLast()
            //   C) Parent has UID                → currentDevice && currentDevice->uid.IsSet()
            //
            // This covers special cases like:
            //   temps::a (explicit empty segment)
            //   temps:a  (implicit skip of placeholder)
            //----------------------------------------------------------------------
            if (dev->uid.NotSet())
            {
                bool isRoot         = (currentDevice == nullptr);
                bool hasMore        = path.hasMore();
                bool parentHasUID   = (currentDevice && currentDevice->uid.IsSet());

                if (isRoot || hasMore || parentHasUID)
                {
                    DeviceFindResult res = dev->findDevice(path, outDevice);
                    if (res == DeviceFindResult::Success)
                        return res;

                    // need to continue search other devices here
                }
            }
        }
        return DeviceFindResult::DeviceNotFound;
    }
/*
    Device* Device::findInArray(Device** devices, int deviceCount, UIDPath& path, Device* currentDevice) {
        if (!devices || deviceCount == 0) return nullptr;
        if (path.empty()) return nullptr;

        HAL_UID currUID;

        // Determine which UID to compare at this level
        if (currentDevice && currentDevice->uid.IsSet()) {
            currUID = path.getNextUID();   // advance for subdevice
        } else {
            currUID = path.getCurrentUID(); // root level or placeholder
        }

        if (currUID.Invalid()) return nullptr;

        Device* indirectMatch = nullptr;

        for (int i = 0; i < deviceCount; i++) {
            Device* dev = devices[i];
            if (!dev) continue;

            if (dev->uid == currUID) {
                if (dev->uidMaxLength == UIDPathMaxLength::One) {
                    if (path.isLast())
                        return dev;  // exact match, path ends here
                    else {
                        GlobalLogger.Error(F("Device UIDPathMaxLength::One cannot resolve multi-segment path"), dev->type);
                        return nullptr; // path has more segments but device can't have children
                    }
                } else {
                    // If a device matched the currUID but couldn't directly resolve the full path,
                    // attempt an indirect lookup via the matched device.
                    indirectMatch = dev->findDevice(path); // recurse into children
                    break; // No need to continue — currUID match is unique
                }
            } else if (dev->uid.NotSet() && !path.isLast()) { // this will only happen on devices where uidMaxLenght>1
                Device* d = dev->findDevice(path); // recurse into placeholder
                if (d) return d; // match allways return valid device
            }
        }
        if (indirectMatch == nullptr) {
            GlobalLogger.Error(F("could not find device: "),path.ToString().c_str());
            return nullptr;
        }
        return indirectMatch;
    }*/

    namespace DeviceConstStrings {
        HAL_JSON_DEVICE_CONST_STR_DEFINE(uid, "\"uid\":\"");
        HAL_JSON_DEVICE_CONST_STR_DEFINE(type, "\"type\":\""); // type allways after uid
        
        HAL_JSON_DEVICE_CONST_STR_DEFINE(pin, ",\"pin\":");
        HAL_JSON_DEVICE_CONST_STR_DEFINE(value, "\"value\":");
        HAL_JSON_DEVICE_CONST_STR_DEFINE(valueStartWithComma, ",\"value\":");
        HAL_JSON_DEVICE_CONST_STR_DEFINE(refreshTimeMs, ",\"refreshTimeMs\":");

    }

} // namespace HAL
