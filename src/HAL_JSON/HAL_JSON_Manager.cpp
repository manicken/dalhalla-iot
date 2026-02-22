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

#include "HAL_JSON_Manager.h"

namespace HAL_JSON {

    Device** Manager::devices = nullptr;
    int Manager::deviceCount = 0;
    //int Manager::reloadVersion = 0;
    bool Manager::reloadQueued = false;
    
    int Manager::DeviceCount() {
        return deviceCount;
    }

   /* int* Manager::ReloadVersionPtr() {
        return &reloadVersion;
    }*/

    bool Manager::setupMgr() {
#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
        if (ReadJSON(String(HAL_JSON_CONFIG_JSON_FILE).c_str()+1) == false) { // remove the leading /
#else
        if (ReadJSON(String(HAL_JSON_CONFIG_JSON_FILE).c_str()) == false) {
#endif
            Serial.println("error happend while reading and parsing config JSON");
            GlobalLogger.printAllLogs(Serial, false);
            return false;
        }
        begin(); // call the begin function on all loaded hal devices
        return true;
    }

    std::string Manager::ToString() {
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

    Device* Manager::CreateDeviceFromJSON(const JsonVariant &jsonObj) {
        const char* type = jsonObj[HAL_JSON_KEYNAME_TYPE].as<const char*>();
        const DeviceRegistryItem& regItem = GetDeviceRegistryItem(type);
        if (regItem.typeName == nullptr) {
            // should never happen as VerifyJson is called before and do actually verify that this function should work
            GlobalLogger.Error(F("CreateDeviceFromJSON - something is very wrong if this happens"));
            return nullptr; // no match
        }

        if (regItem.def.Create_Function == nullptr) {
            GlobalLogger.Error(F("CreateDeviceFromJSON - Create_Function == nullptr - something is very wrong if this happens"));
            return nullptr; // should never happen as VerifyJson is called before and do actually verify that this pointer do point to something
        }
        return regItem.def.Create_Function(jsonObj, regItem.typeName);
    }
    bool Manager::VerifyDeviceJson(const JsonVariant &jsonObj) {
        
        if (!ValidateJsonStringField(jsonObj, HAL_JSON_KEYNAME_TYPE)) { SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_MGR_VERIFY_DEVICE); return false; }

        const char* type = jsonObj[HAL_JSON_KEYNAME_TYPE].as<const char*>();

        const DeviceRegistryItem& regItem = GetDeviceRegistryItem(type);
        if (regItem.typeName == nullptr) {
            GlobalLogger.Error(F("VerifyDeviceJson - could not find type:"),type);
            return false;
        }

        if (regItem.def.useRootUID == UseRootUID::Mandatory)
            if (!ValidateJsonStringField(jsonObj, HAL_JSON_KEYNAME_UID)) { SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_MGR_VERIFY_DEVICE); return false; }

        if (regItem.def.Verify_JSON_Function == nullptr){ GlobalLogger.Error(F("Verify_JSON_Function missing for:"),type); return false; }
        if (regItem.def.Create_Function == nullptr){ GlobalLogger.Error(F("Create_Function missing for:"), type); return false; } // skip devices that dont have this defined

        return regItem.def.Verify_JSON_Function(jsonObj);

    }

    void Manager::CleanUp() {
        //printf("\n&&&&&&&&&&&&&&&&&&&&&&&& CLEANUP OF LOADED DEVICES &&&&&&&&&&&&&&&&&&&&&&\n");
        // cleanup of prev device list if existent
        if (devices != nullptr) {
            for (int i=0;i<HAL_JSON::Manager::deviceCount;i++) {
                if (devices[i] != nullptr) {
                    delete devices[i];
                    devices[i] = nullptr;
                }
            }
            delete[] devices;
            devices = nullptr;
        }
        HAL_JSON::Manager::deviceCount = 0;
    }

    bool Manager::ParseJSON(const JsonArray &jsonArray) {
        //Serial.println("PArse json thianasoidnoasidnasoidnsaiodnsaodinasdoiandoisandiosndoiasnd");
        uint32_t deviceCount = 0;
        int arraySize = jsonArray.size();
        bool* validDevices = new bool[arraySize]; // dont' forget the delete[] call at end of function
        GPIO_manager::ClearAllReservations(); // when devices are verified they also reservate the pins to include checks for duplicate use

        // First pass: count valid entries
        for (int i=0;i<arraySize;i++) {

            JsonVariant jsonItem = jsonArray[i];

            if (IsConstChar(jsonItem) == true) { validDevices[i] = false;  continue; } // comment item

            if (Device::DisabledInJson(jsonItem) == true) { validDevices[i] = false;  continue; } // disabled

            bool valid = VerifyDeviceJson(jsonItem);

            validDevices[i] = valid;
            if (valid == false) HAL_JSON_VALIDATE_IN_LOOP_FAIL_OPERATION; // could either be continue; or return false depending if strict mode is on/off
            deviceCount++;
        }
        CleanUp();
        
        
        if (deviceCount == 0) {
            GlobalLogger.Error(F("The loaded JSON cfg does not contain any valid devices!\n" 
                                 "Hint: Check that all entries have 'type' and 'uid' fields, and match known types."));
            return false;
        }
        printf("\nTrying to allocate for %d devices\n", deviceCount);

        // Allocate space for all devices
        devices = new Device*[deviceCount]();
        
        if (devices == nullptr) {
            GlobalLogger.Error(F("Failed to allocate device array"));
            return false;
        }
        printf("\nOK\n");
        HAL_JSON::Manager::deviceCount = deviceCount;

        GPIO_manager::ClearAllReservations(); 
        // Second pass: actually create and store devices
        uint32_t index = 0;
        for (int i=0;i<arraySize;i++) {
            JsonVariant jsonItem = jsonArray[i];
            //if (VerifyDeviceJson(jsonItem) == false) continue; // ************************************************************ now as we dont run this again the pins are not allocated anymore but we don't really need to take care of that as it's part of the validate device check anyway
            if (validDevices[i] == false) continue;
            devices[index++] = CreateDeviceFromJSON(jsonItem);
        }
        std::string devCountStr = std::to_string(deviceCount);
        GlobalLogger.Info(F("Created devices: "), devCountStr.c_str());
        delete[] validDevices; // free memory
        return true;
    }

    DeviceFindResult Manager::findDevice(UIDPath& path, Device*& outDevice) {
        path.reset(); // ensure to be at root level
        return Device::findInArray(devices, deviceCount, path, nullptr, outDevice);
    }
    /*
    HALOperationResult Manager::GetDeviceEvent(ZeroCopyString zcStrUidPathAndFuncName, Device::DeviceEvent** deviceEventOut) {
        ZeroCopyString zcFuncName = zcStrUidPathAndFuncName.SplitOffTail('@');
        UIDPath uidPath(zcStrUidPathAndFuncName);
        Device* deviceOut = nullptr;
        DeviceFindResult devFindRes = Manager::findDevice(uidPath, deviceOut);

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
    HALOperationResult Manager::GetDeviceEvent(ZeroCopyString zcStrUidPathAndFuncName, Device::DeviceEvent** deviceEventOut)
    {
        ZeroCopyString zcFuncName = zcStrUidPathAndFuncName.SplitOffTail('#');
        UIDPath uidPath(zcStrUidPathAndFuncName);

        Device* deviceOut = nullptr;
        DeviceFindResult findRes = Manager::findDevice(uidPath, deviceOut);

        if (findRes != DeviceFindResult::Success) {
            if (deviceEventOut) {
                *deviceEventOut = nullptr;
            }
            return HALOperationResult::DeviceNotFound;
        }

        // Forward directly to device
        return deviceOut->Get_DeviceEvent(zcFuncName, deviceEventOut);
    }
    HALOperationResult Manager::ValidateDeviceEvent(ZeroCopyString zcStrUidPathAndFuncName) {
        return GetDeviceEvent(zcStrUidPathAndFuncName, nullptr);
    }

    bool Manager::ReadJSON(const char* path) {
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
        if (!jsonDoc.is<JsonArray>())
        {
            delete[] jsonBuffer;
            GlobalLogger.Error(F("jsonDoc root is not a JsonArray"));
            return false;
        }
        const JsonArray& jsonItems = jsonDoc.as<JsonArray>();

        bool parseOk = ParseJSON(jsonItems);

        delete[] jsonBuffer;

        return parseOk;
    }
    void Manager::begin() {
        for (int i=0;i<deviceCount;i++) {
            Device* device = devices[i];
            if (device == nullptr) continue;
            device->begin();
            delay(0); // give time to RTOS and WiFi tasks
        }
    }

    void Manager::loop() {
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
    /*void Manager::TEST() {

        ZeroCopyString zcPath = "1WTG";
        UIDPath path(zcPath);
        Device* device = nullptr;
        DeviceFindResult devFindRes = Manager::findDevice(path, device);
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
        DeviceFindResult devFindRes2 = Manager::findDevice(path2, device2);
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