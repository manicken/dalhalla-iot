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

#include <cstdint>
#include <cstring>
#include <string>

//#include <Arduino.h> // Needed for String class

#include <DALHAL/API/DALHAL_StringBuilderStreamer.h>

#include <ArduinoJson.h>
#include <DALHAL/Core/Types/DALHAL_Value.h>
#include <DALHAL/Core/Types/DALHAL_UID_Path.h>
#include <DALHAL/Core/Types/DALHAL_OperationResult.h>
#include <DALHAL/Core/Types/DALHAL_Operations.h>
#include <DALHAL/Support/DALHAL_DeleterTemplate.h>
#include <DALHAL/Core/Reactive/DALHAL_ReactiveEvent.h>

#include <DALHAL/Core/Types/DALHAL_DeviceFunctionTypes.h> // TODO remove when all getter functions are removed from all devices

namespace DALHAL {

    // forward declaration
    namespace Registry { struct DefineBase; }
    
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
        
        
    public:
        const char* const Type;

        Device(const char* const type);
        virtual ~Device();

        HAL_UID uid;

        virtual const Registry::DefineBase* GetRegistryDefine() = 0;
        
        virtual HALOperationResult read(HALValue& val);
        virtual HALOperationResult write(const HALValue& val);
        virtual HALOperationResult read(const HALValue& bracketSubscriptVal, HALValue& val);
        virtual HALOperationResult write(const HALValue& bracketSubscriptVal, const HALValue& val);
        /* TODO START: remove following functions in favor of function table */
        /* they are only used by the CLI API so it should be easy */
        /* yes they cannot be used by the script engine or any other runtime stuff as
         * strings should not be stored and/or parsed at runtime unless there is no other option
         * actually the use lockup internally so moving that into CLI API would save some flash
         * and also make the code cleaner as there is not much to reason about
         */
        virtual HALOperationResult read(const HALReadStringRequestValue& val);
        virtual HALOperationResult write(const HALWriteStringRequestValue& val);
        virtual HALOperationResult read(const HALReadValueByCmd& val);
        virtual HALOperationResult write(const HALWriteValueByCmd& val);
   
        /** 
         * Executes a device action with a provided command string, 
         * only used when doing remote cmd:s, i.e. not used by script.
         * a special note: Home Assistant currently is using it to decode 
         * state update
         */
        
        virtual HALOperationResult exec(const ZeroCopyString& cmd);
        /* TODO END: remove following functions in favor of function table */

        virtual HALValue* GetValueDirectAccessPtr();

        virtual HALOperationResult Get_ReactiveEvent(ZeroCopyString& zcFuncName, ReactiveEvent** reactiveEventOut);
        
        /** called regulary from the main loop */
        virtual void loop();
        /** called when all hal devices has been loaded */
        virtual void begin();
        /** used to find sub/leaf devices @ "group devices" */
        virtual DeviceFindResult findDevice(UIDPath& path, Device*& outDevice);

        /** Executes a device action that requires no parameters. */
        virtual HALOperationResult exec();
        
        //virtual String ToString();
        virtual void PrintTo(StringBuilderStreamer& sbs);

        static bool DisabledOrCommentItem(const JsonVariant& jsonObj);
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
