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

#pragma once

#include <cstdint>
#include <cstring>
#include <string>

#include <Arduino.h> // Needed for String class

#include <ArduinoJson.h>
#include "../Types/DALHAL_Value.h"
#include "../Types/DALHAL_UID_Path.h"
#include "../Types/DALHAL_OperationResult.h"
#include "../Types/DALHAL_Operations.h"

namespace DALHAL {
    
    enum class DeviceFindResult {
        Success,
        DeviceNotFound,
        PathTooDeep,
        SubDevicesNotSupported,
        SubDeviceListEmpty,
        EmptyUIDPath,
        InvalidUID
    };
    const char* DeviceFindResultToString(DeviceFindResult res);

    class Device {
        
    protected:
        Device() = delete;
        Device(Device&) = delete;
        const char* type;
        
    public:
        const char* GetType();
        
        using Exec_FuncType = HALOperationResult (*)(Device*);
        using ReadToHALValue_FuncType = HALOperationResult (*)(Device* device, HALValue& outValue);
        using WriteHALValue_FuncType = HALOperationResult (*)(Device* device, const HALValue& value);
        using BracketOpRead_FuncType = HALOperationResult (*)(Device* device, const HALValue& subscriptValue, HALValue& outValue);
        using BracketOpWrite_FuncType = HALOperationResult (*)(Device* device, const HALValue& subscriptValue, const HALValue& value);

        class DeviceEvent {
        public:
            template<typename T>
            static void DeleteAs(void* ptr) {
                delete static_cast<T*>(ptr);
            }
            using CheckFn  = bool (*)(void*);
            using DeleteFn = void (*)(void*);

        private:
            CheckFn checkFn;
            DeleteFn deleteFn;
            void* context;
        public:
            DeviceEvent() = delete;
            DeviceEvent(DeviceEvent&) = delete;
            DeviceEvent(CheckFn checkFn, DeleteFn deleteFn, void* context);

            ~DeviceEvent();

            inline bool CheckForEvent() {
                return checkFn(context);
            }
        };

        Device(const char* type);
        virtual ~Device();

        HAL_UID uid;
        
        virtual HALOperationResult read(HALValue& val);
        virtual HALOperationResult write(const HALValue& val);
        virtual HALOperationResult read(const HALValue& bracketSubscriptVal, HALValue& val);
        virtual HALOperationResult write(const HALValue& bracketSubscriptVal, const HALValue& val);
        virtual HALOperationResult read(const HALReadStringRequestValue& val);
        virtual HALOperationResult write(const HALWriteStringRequestValue& val);
        virtual HALOperationResult read(const HALReadValueByCmd& val);
        virtual HALOperationResult write(const HALWriteValueByCmd& val);
        virtual ReadToHALValue_FuncType GetReadToHALValue_Function(ZeroCopyString& zcFuncName);
        virtual WriteHALValue_FuncType GetWriteFromHALValue_Function(ZeroCopyString& zcFuncName);
        virtual Exec_FuncType GetExec_Function(ZeroCopyString& zcFuncName);
        virtual HALValue* GetValueDirectAccessPtr();

        virtual BracketOpRead_FuncType GetBracketOpRead_Function(ZeroCopyString& zcFuncName);
        virtual BracketOpWrite_FuncType GetBracketOpWrite_Function(ZeroCopyString& zcFuncName);

        virtual HALOperationResult Get_DeviceEvent(ZeroCopyString& zcFuncName, Device::DeviceEvent** deviceEventOut);
        //virtual HALOperationResult EventCheck(uint32_t lastSeen);
        
        
        /** called regulary from the main loop */
        virtual void loop();
        /** called when all hal devices has been loaded */
        virtual void begin();
        /** used to find sub/leaf devices @ "group devices" */
        virtual DeviceFindResult findDevice(UIDPath& path, Device*& outDevice);

        /** Executes a device action that requires no parameters. */
        virtual HALOperationResult exec();
        /** Executes a device action with a provided command string, only used when doing remote cmd:s, i.e. not used by script. */
        virtual HALOperationResult exec(const ZeroCopyString& cmd);

        virtual String ToString();

        static bool DisabledInJson(const JsonVariant& jsonObj);

        static DeviceFindResult findInArray(Device** devices, int deviceCount, UIDPath& path, Device* currentDevice, Device*& outDevice);
    };

    
    
#if defined(ESP32)
//#define DALHAL_DEVICE_CONST_STRINGS_USE_F_PREFIX
#endif

#ifdef DALHAL_DEVICE_CONST_STRINGS_USE_F_PREFIX
#define DALHAL_DEVICE_CONST_STR_DECLARE(name) extern const __FlashStringHelper* name
#define DALHAL_DEVICE_CONST_STR_DEFINE(name, value) const __FlashStringHelper* name = F(value)
#else
#define DALHAL_DEVICE_CONST_STR_DECLARE(name) extern const char* name
#define DALHAL_DEVICE_CONST_STR_DEFINE(name, value) const char* name = value
#endif

    namespace DeviceConstStrings {
        DALHAL_DEVICE_CONST_STR_DECLARE(uid);
        /** "\"type\":\"" */
        DALHAL_DEVICE_CONST_STR_DECLARE(type);
        DALHAL_DEVICE_CONST_STR_DECLARE(pin);
        DALHAL_DEVICE_CONST_STR_DECLARE(value);
        DALHAL_DEVICE_CONST_STR_DECLARE(valueStartWithComma);
        DALHAL_DEVICE_CONST_STR_DECLARE(refreshTimeMs);

    }

    
} // namespace HAL
