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

#include "DALHAL_Display_SSD1306.h"

#include <ArduinoJson.h>

#include <DALHAL/Core/Device/DALHAL_Device.h>
#include <DALHAL/Core/JsonConfig/DALHAL_JSON_Config_Strings.h>

#include <DALHAL/Support/DALHAL_Logger.h>
#include <DALHAL/Core/JsonConfig/DALHAL_ArduinoJSON_ext.h>
#include <DALHAL/Core/Types/DALHAL_Registry.h>

#include "DALHAL_Display_SSD1306_JSON_Schema.h"

namespace DALHAL {

    constexpr I2C_RegistryDefine Display_SSD1306::RegistryDefine = {
        Create,
        &JsonSchema::Display_SSD1306,
        DALHAL_REACTIVE_EVENT_TABLE(DISPLAY_SSD1306),
        HasAddress
    };

    bool Display_SSD1306::HasAddress(uint8_t addr) {
        return addr == 0x3C || addr == 0x3D; 
    }

    Device* Display_SSD1306::Create(DeviceCreateContext& context) {
        return new Display_SSD1306(static_cast<I2C_Master_CreateFunctionContext&>(context));
    }
    
    Display_SSD1306::Display_SSD1306(I2C_Master_CreateFunctionContext& context) : Display_SSD1306_DeviceBase(context.deviceType) {
        const JsonVariant& jsonObj = *(context.jsonObjItem);
        const char* uidStr = GetAsConstChar(jsonObj,DALHAL_KEYNAME_UID);
        uid = encodeUID(uidStr);

        uint32_t width = GetAsUINT32(jsonObj, "width");
        uint32_t height = GetAsUINT32(jsonObj, "height");
        const char* addrStr = GetAsConstChar(jsonObj, "addr");
        uint8_t addr = static_cast<uint8_t>(std::strtoul(addrStr, nullptr, 16));
        uint8_t textSize = GetAsUINT8(jsonObj, "textsize");
        if (textSize == 0) textSize = 1;

        display = new Adafruit_SSD1306(width, height, &(context.wire), -1); // -1 = no reset pin

        delay(200);
        if (display->begin(SSD1306_SWITCHCAPVCC, addr))
        {
            display->clearDisplay();
            display->setTextSize(1);
            display->setTextColor(SSD1306_WHITE);
            display->setCursor(0,0);
            //display.println(F("Hello ESP32!"));
            display->display(); // <--- push buffer to screen
        }

        const JsonArray items = jsonObj[DALHAL_KEYNAME_ITEMS].as<JsonArray>();

        int itemCount = items.size();
        // first pass count valid items
        size_t validItemCount = 0;
        for (int i=0;i<itemCount;i++) {
            const JsonVariant& item = items[i];
            if (IsConstChar(item) == true) { continue; } // comment item
            if (Device::DisabledInJson(item) == true) { continue; } // disabled
            validItemCount++;
        }
        // second pass actually create the devices
        elementCount = validItemCount;
        elements = new Device*[validItemCount](); /*Display_SSD1306_Element*/
        int index = 0;
        DeviceCreateContext createContext;
        for (int i=0;i<itemCount;i++) {
            const JsonVariant& item = items[i];
            if (IsConstChar(item) == true) continue;
            if (Device::DisabledInJson(item) == true) continue;

            createContext.jsonObjItem = &item;
            createContext.deviceType = "I2C_DISP_SSD1306_ELM";
            elements[index++] = new Display_SSD1306_Element(createContext);
        }
    }

    Display_SSD1306::~Display_SSD1306() {
        if (elements != nullptr) {
            for (int i=0;i<elementCount;i++) {
                delete elements[i];
                elements[i] = nullptr;
            }
            delete[] elements;
            elements = nullptr;
            elementCount = 0;
        }
        delete display;
    }

    String Display_SSD1306::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += this->Type;
        
        return ret;
    }

    DeviceFindResult Display_SSD1306::findDevice(UIDPath& path, Device*& outDevice) {
        return Device::findInArray(elements, elementCount, path, this, outDevice);
    }

    HALOperationResult Display_SSD1306::write(const HALWriteStringRequestValue& val) {
        ZeroCopyString zcData = val.value;
        //printf("\nDisplay_SSD1306::write data:%s\n", zcData.ToString().c_str());
        ZeroCopyString zcCmd = zcData.SplitOffHead('/');
        
        if (zcCmd == "text") {
            //printf("\nDisplay_SSD1306::write text:%s\n", zcData.ToString().c_str());
            display->write(zcData.start, zcData.Length());
        }
        else if (zcCmd == "print") {
            //printf("\nDisplay_SSD1306::write print:%s\n", zcData.ToString().c_str());
            display->write(zcData.start, zcData.Length());
            display->display();
        }
        else if (zcCmd == "cursor") {
            ZeroCopyString zcXstr = zcData.SplitOffHead('/');
            int32_t x = 0, y = 0;
            zcXstr.ConvertTo_int32(x);
            zcData.ConvertTo_int32(y);
            display->setCursor(x,y);
        } else if (zcCmd == "clear") {
            display->clearDisplay();
        } else if (zcCmd == "update") {
            display->display();
        } else {
            return HALOperationResult::UnsupportedCommand;
        }
#if HAS_REACTIVE_WRITE(DISPLAY_SSD1306)
        triggerWrite();
#endif
        return HALOperationResult::Success;
    }

    void Display_SSD1306::loop() {
        display->clearDisplay();
        for (int i=0;i<elementCount;i++) {
            Display_SSD1306_Element* elPtr = static_cast<Display_SSD1306_Element*>(elements[i]);
            if (elPtr == nullptr) continue;
            Display_SSD1306_Element& el = *elPtr;
            display->setCursor(el.xPos, el.yPos);
            display->print(el.label.c_str());

            if (el.cdaSource != nullptr) {
                el.cdaSource->ReadSimple(el.val);
            }
            HALValue::Type t = el.val.getType();
            if (t == HALValue::Type::FLOAT)
                display->print(el.val.asFloat());
            else if (t == HALValue::Type::UINT)
                display->print(el.val.asUInt());
            else if (t == HALValue::Type::INT)
                display->print(el.val.asInt());
        }
        display->display(); // update all in one go
    }
}