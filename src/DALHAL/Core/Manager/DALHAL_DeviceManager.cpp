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

#include "DALHAL_DeviceManager.h"

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>
#include <DALHAL/Core/Types/DALHAL_Registry.h>
#include <DALHAL/Devices/_Registry/DALHAL_DevicesRegistry.h>

#include <DALHAL/Support/DALHAL_Logger.h>
#include <DALHAL/Core/JsonConfig/DALHAL_ArduinoJSON_ext.h>
#include <DALHAL/Core/Manager/DALHAL_GPIO_Manager.h>

#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Schema_Validator.h>

namespace DALHAL {

    Device** DeviceManager::devices = nullptr;
    int DeviceManager::deviceCount = 0;
    
    int DeviceManager::DeviceCount() {
        return deviceCount;
    }

    bool DeviceManager::setupMgr() {
#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
        if (ReadJSON(String(DALHAL_CONFIG_JSON_FILE).c_str()+1) == false) { // remove the leading /
#else
        if (ReadJSON(String(DALHAL_CONFIG_JSON_FILE).c_str()) == false) {
#endif
            Serial.println("error happend while reading and parsing config JSON");
            GlobalLogger.printAllLogs(Serial, false);
            return false;
        }
        begin(); // call the begin function on all loaded hal devices
        return true;
    }

    std::string DeviceManager::ToString() {
        std::string ret;
        ret += "\"deviceCount\":" + std::to_string(deviceCount); 
        ret += ",\"devices\":[";
        for (int i=0;i<deviceCount;i++) {
            ret += "{";
            ret += devices[i]->ToString().c_str();
            ret += "}";
            if (i<deviceCount-1) ret += ",";
        }
        ret += "]";
        return ret;
    }

    Device* DeviceManager::CreateDeviceFromJSON(const JsonVariant& jsonObj) {
        const char* type = jsonObj[DALHAL_KEYNAME_TYPE].as<const char*>();
        const Registry::Item& regItem = Registry::GetItem(RootDevicesRegistry, type);
        if (regItem.typeName == nullptr) {
            // should never happen as VerifyJson is called before and do actually verify that this function should work
            GlobalLogger.Error(F("CreateDeviceFromJSON - something is very wrong if this happens"));
            return nullptr; // no match
        }

        if (regItem.def->Create_Function == nullptr) {
            GlobalLogger.Error(F("CreateDeviceFromJSON - Create_Function == nullptr - something is very wrong if this happens"));
            return nullptr; // should never happen as VerifyJson is called before and do actually verify that this pointer do point to something
        }
        DeviceCreateContext createContext;
        createContext.jsonObjItem = &jsonObj;
        createContext.deviceType = regItem.typeName;
        return regItem.def->Create_Function(createContext);
    }

    void DeviceManager::CleanUp() {
        //printf("\n&&&&&&&&&&&&&&&&&&&&&&&& CLEANUP OF LOADED DEVICES &&&&&&&&&&&&&&&&&&&&&&\n");
        // cleanup of prev device list if existent
        if (devices != nullptr) {
            for (int i=0;i<DALHAL::DeviceManager::deviceCount;i++) {
                if (devices[i] != nullptr) {
                    delete devices[i];
                    devices[i] = nullptr;
                }
            }
            delete[] devices;
            devices = nullptr;
        }
        DALHAL::DeviceManager::deviceCount = 0;
    }

    bool DeviceManager::ParseJSON(const JsonVariant &jsonArray) {
        //Serial.println("PArse json thianasoidnoasidnasoidnsaiodnsaodinasdoiandoisandiosndoiasnd");
        GPIO_manager::ClearAllReservations(); // when devices are verified they also reservate the pins to include checks for duplicate use
        bool anyError = false;
        JsonSchema::validateFromRegister(jsonArray, RootDevicesRegistry, anyError);
        if (anyError) {
            GlobalLogger.Error(F("The loaded JSON cfg contains errors"));
            GlobalLogger.setLastEntrySource("DeviceManager::ParseJSON");
            return false;
        }

        // First pass: count enabled/(non comment) entries
        uint32_t deviceCount = 0;
        int arraySize = jsonArray.size();
        for (int i=0;i<arraySize;i++) {
            JsonVariant jsonItem = jsonArray[i];
            if (IsConstChar(jsonItem) == true) { continue; } // comment item
            if (Device::DisabledInJson(jsonItem) == true) { continue; } // disabled
            deviceCount++;
        }
        
        if (deviceCount == 0) {
            GlobalLogger.Error(F("The loaded JSON cfg does not contain any enabled/(non comment) items!"));
            GlobalLogger.setLastEntrySource("DeviceManager::ParseJSON");
            return false;
        }
        
        // delete/cleanup prev configuration if any
        CleanUp();
        printf("\nTrying to allocate for %d devices\n", deviceCount);
        
        // Allocate space for all devices
        devices = new (std::nothrow) Device*[deviceCount]();
        
        if (devices == nullptr) {
            GlobalLogger.Error(F("Failed to allocate device array"));
            return false;
        }
        printf("\nOK\n");
        DALHAL::DeviceManager::deviceCount = deviceCount;

        GPIO_manager::ClearAllReservations(); 
        // Second pass: actually create and store devices
        uint32_t index = 0;
        for (int i=0;i<arraySize;i++) {
            JsonVariant jsonItem = jsonArray[i];
            if (IsConstChar(jsonItem) == true) { continue; } // comment item
            if (Device::DisabledInJson(jsonItem) == true) { continue; } // disabled
            devices[index++] = CreateDeviceFromJSON(jsonItem);
        }
        std::string devCountStr = std::to_string(deviceCount);
        GlobalLogger.Info(F("Created devices: "), devCountStr.c_str());
        return true;
    }

    DeviceFindResult DeviceManager::findDevice(UIDPath& path, Device*& outDevice) {
        path.reset(); // ensure to be at root level
        return Device::findInArray(devices, deviceCount, path, nullptr, outDevice);
    }
    /*
    HALOperationResult DeviceManager::GetDeviceEvent(ZeroCopyString zcStrUidPathAndFuncName, Device::DeviceEvent** deviceEventOut) {
        ZeroCopyString zcFuncName = zcStrUidPathAndFuncName.SplitOffTail('@');
        UIDPath uidPath(zcStrUidPathAndFuncName);
        Device* deviceOut = nullptr;
        DeviceFindResult devFindRes = DeviceManager::findDevice(uidPath, deviceOut);

        if (devFindRes != DeviceFindResult::Success) {
            if (deviceEventOut) {
                *deviceEventOut = nullptr;
            }
            return HALOperationResult::DeviceNotFound;
        }

        Device::DeviceEvent* deviceEventTemp = nullptr;
        HALOperationResult res = deviceOut->Get_DeviceEvent(zcFuncName, &deviceEventTemp);

        if (res != HALOperationResult::Success) {
            if (deviceEventOut) {
                *deviceEventOut = nullptr;
            }
            return res;
        }

        // Success case
        if (deviceEventOut) {
            *deviceEventOut = deviceEventTemp;   // transfer ownership
        } else {
            delete deviceEventTemp;             // test mode → auto delete
        }

        return HALOperationResult::Success;
    }*/
    HALOperationResult DeviceManager::GetDeviceEvent(ZeroCopyString zcStrUidPathAndFuncName, ReactiveEvent** reactiveEventOut)
    {
        ZeroCopyString zcFuncName = zcStrUidPathAndFuncName.SplitOffTail('#');
        UIDPath uidPath(zcStrUidPathAndFuncName);

        Device* deviceOut = nullptr;
        DeviceFindResult findRes = DeviceManager::findDevice(uidPath, deviceOut);

        if (findRes != DeviceFindResult::Success) {
            if (reactiveEventOut) {
                *reactiveEventOut = nullptr;
            }
            return HALOperationResult::DeviceNotFound;
        }

        // Forward directly to device
        return deviceOut->Get_ReactiveEvent(zcFuncName, reactiveEventOut);
    }
    HALOperationResult DeviceManager::ValidateDeviceEvent(ZeroCopyString zcStrUidPathAndFuncName) {
        return GetDeviceEvent(zcStrUidPathAndFuncName, nullptr);
    }

    bool DeviceManager::ReadJSON(const char* path) {
        if (path == nullptr) {
            GlobalLogger.Error(F("ReadJSON - path cannot be empty "));
            return false;
        }
        if (LittleFS.exists(path) == false) {
            GlobalLogger.Error(F("ReadJSON - cfg file did not exist: "),path);
            return false;
        }

        char* jsonBuffer = nullptr;
        size_t fileSize=0;
        const char* filePath = path;
        if (LittleFS_ext::load_text_file(filePath, &jsonBuffer, &fileSize) != LittleFS_ext::FileResult::Success)
        {
            GlobalLogger.Error(F("ReadJSON - error could not load json file: "),path);
            return false;
        }
#if defined(ESP8266) || defined(ESP32)
        size_t jsonDocBufferSize = (size_t)((float)fileSize * 2.0f);
#elif defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
        size_t jsonDocBufferSize = fileSize * 10; // very safe mem
#else
        size_t jsonDocBufferSize = (size_t)((float)fileSize * 1.5f);
#endif
        //size_t requiredSize = measureJson((JsonVariantConst)jsonBuffer);
        DynamicJsonDocument jsonDoc(jsonDocBufferSize);
        DeserializationError error = deserializeJson(jsonDoc, jsonBuffer);
        if (error)
        {
            delete[] jsonBuffer;
            GlobalLogger.Error(F("ReadJSON - deserialization failed: "), error.c_str());
            return false;
        }

        std::string memUsage = std::to_string(jsonDoc.memoryUsage()) + " of " + std::to_string(jsonDoc.capacity());
        GlobalLogger.Info(F("jsonDoc.memoryUsage="), memUsage.c_str());

        
        /*if (!jsonDoc.is<JsonArray>())
        {
            delete[] jsonBuffer;
            GlobalLogger.Error(F("jsonDoc root is not a JsonArray"));
            return false;
        }
        const JsonArray& jsonItems = jsonDoc.as<JsonArray>();
*/
        bool parseOk = ParseJSON(jsonDoc);

        delete[] jsonBuffer;

        return parseOk;
    }
    void DeviceManager::begin() {
        for (int i=0;i<deviceCount;i++) {
            Device* device = devices[i];
            if (device == nullptr) continue;
            device->begin();
            delay(0); // give time to RTOS and WiFi tasks
        }
    }

    void DeviceManager::loop() {
        if ((devices == nullptr) || (deviceCount == 0)) return;

        for (int i=0;i<deviceCount;i++) {
            Device* device = devices[i];
            if (device == nullptr) continue;
            device->loop();
            delay(0); // give time to RTOS and WiFi tasks
        }
    }
    /** 
     * the following is not intended to be used  
     * it's just to check that everything is correct
     * in the future it could be a real TEST function
    */
    /*void DeviceManager::TEST() {

        ZeroCopyString zcPath = "1WTG";
        UIDPath path(zcPath);
        Device* device = nullptr;
        DeviceFindResult devFindRes = DeviceManager::findDevice(path, device);
        if (devFindRes != DeviceFindResult::Success) {
            std::string msg = "\"error\":\""+std::string(DeviceFindResultToString(devFindRes))+": " + zcPath.ToString() + "\"";
            Serial.println(msg.c_str());
            
        } else {
            std::string result;
            ZeroCopyString cmd("getDevices");
            HALReadStringRequestValue strVal = {cmd, result};
            //HALReadStringRequest req{path, strVal};
            if (device->read(strVal) == HALOperationResult::Success) {

                Serial.println(strVal.out_value.c_str());
            }
        }

        ZeroCopyString zcPath2 = "1WTG:D2";
        UIDPath path2(zcPath2);
        Device* device2 = nullptr;
        DeviceFindResult devFindRes2 = DeviceManager::findDevice(path2, device2);
        if (devFindRes2 != DeviceFindResult::Success) {
            std::string msg = "\"error\":\""+std::string(DeviceFindResultToString(devFindRes2))+": " + zcPath2.ToString() + "\"";
            Serial.println(msg.c_str());
            
        } else {
            HALValue value;
            //HALReadRequest req2(path2, value); // obsolete
            if (device2->read(value) == HALOperationResult::Success) {
                Serial.println(value.asFloat());
            }
        }
    }*/
}