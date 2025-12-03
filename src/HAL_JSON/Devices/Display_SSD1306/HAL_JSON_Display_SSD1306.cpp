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

#include "HAL_JSON_Display_SSD1306.h"

#include "../../../Support/Logger.h"
#include "../../HAL_JSON_Device.h"
#include "../../HAL_JSON_Device_GlobalDefines.h"
#include "../../HAL_JSON_ArduinoJSON_ext.h"
#include "../../../Support/CharArrayHelpers.h"

namespace HAL_JSON {

    bool Display_SSD1306::HasAddress(uint8_t addr) {
        return addr == 0x3C || addr == 0x3D; 
    }

    Device* Display_SSD1306::Create(const JsonVariant &jsonObj, const char* type, TwoWire& wire) {
        return new Display_SSD1306(jsonObj, type, wire);
    }
    
    Display_SSD1306::Display_SSD1306(const JsonVariant &jsonObj, const char* type, TwoWire& wire) : Device(type) {

        const char* uidStr = GetAsConstChar(jsonObj,HAL_JSON_KEYNAME_UID);
        uid = encodeUID(uidStr);

        uint32_t width = GetAsUINT32(jsonObj, "width");
        uint32_t height = GetAsUINT32(jsonObj, "height");
        const char* addrStr = GetAsConstChar(jsonObj, "addr");
        uint8_t addr = static_cast<uint8_t>(std::strtoul(addrStr, nullptr, 16));
        uint8_t textSize = GetAsUINT8(jsonObj, "textsize");
        if (textSize == 0) textSize = 1;

        display = new Adafruit_SSD1306(width, height, &Wire, -1); // -1 = no reset pin
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

        const JsonArray items = jsonObj[HAL_JSON_KEYNAME_ITEMS].as<JsonArray>();

        int itemCount = items.size();
        bool* validItems = new bool[itemCount];
        // first pass count valid items
        size_t validItemCount = 0;
        for (int i=0;i<itemCount;i++) {
            const JsonVariant item = items[i];
            if (IsConstChar(item) == true) { validItems[i] = false; continue; }// comment item
            if (Device::DisabledInJson(item) == true) { validItems[i] = false; continue; } // disabled

            if (Display_SSD1306_Element::VerifyJSON(item) == false) { validItems[i] = false; continue; }
            validItemCount++;
            validItems[i] = true;
        }
        // second pass actually create the devices
        elementCount = validItemCount;
        elements = new Display_SSD1306_Element*[validItemCount]();
        int index = 0;
        for (int i=0;i<itemCount;i++) {
            const JsonVariant item = items[i];
            if (validItems[i] == false) continue;
            
            elements[index++] = new Display_SSD1306_Element(item, "I2C_DISP_SSD1306_ELM");
        }
        delete[] validItems;
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

    bool Display_SSD1306::VerifyJSON(const JsonVariant &jsonObj) {
        if (!ValidateJsonStringField(jsonObj, HAL_JSON_KEYNAME_UID)){ SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_DISPLAY_SSD1306_VERIFY_JSON); return false; }

        bool anyError = false;
        
        anyError = false == ValidateUINT32(jsonObj, "width");
        anyError = false == ValidateUINT32(jsonObj, "height");
        anyError = false == ValidateJsonStringField(jsonObj, "addr");
        

        if (jsonObj.containsKey(HAL_JSON_KEYNAME_ITEMS) == false) {
            GlobalLogger.Error(HAL_JSON_ERR_MISSING_KEY(HAL_JSON_KEYNAME_ITEMS));
            SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_DISPLAY_SSD1306_VERIFY_JSON);
            return false;
        }
        if (jsonObj[HAL_JSON_KEYNAME_ITEMS].is<JsonArray>() == false) {
            GlobalLogger.Error(HAL_JSON_ERR_VALUE_TYPE(HAL_JSON_KEYNAME_ITEMS " not array"));
            SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_DISPLAY_SSD1306_VERIFY_JSON);
            return false;
        }
        const JsonArray items = jsonObj[HAL_JSON_KEYNAME_ITEMS].as<JsonArray>();
        if (items.size() == 0) {
            GlobalLogger.Error(HAL_JSON_ERR_ITEMS_EMPTY());
            SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_DISPLAY_SSD1306_VERIFY_JSON);
            return false;
        }
        int itemCount = items.size();
        size_t validItemCount = 0;
        for (int i=0;i<itemCount;i++) {
            const JsonVariant item = items[i];
            if (IsConstChar(item) == true) continue; // comment item
            if (Device::DisabledInJson(item) == true) continue; // disabled
            
            if (Display_SSD1306_Element::VerifyJSON(item) == false) HAL_JSON_VALIDATE_IN_LOOP_FAIL_OPERATION;
            validItemCount++;

        }
        if (validItemCount == 0) {
            GlobalLogger.Error(HAL_JSON_ERR_ITEMS_NOT_VALID(""));
            SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_DISPLAY_SSD1306_VERIFY_JSON);
            return false;
        }

        return anyError == false;
    }

    String Display_SSD1306::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += type;
        
        return ret;
    }

    DeviceFindResult Display_SSD1306::findDevice(UIDPath& path, Device*& outDevice) {
        return Device::findInArray(reinterpret_cast<Device**>(elements), elementCount, path, this, outDevice);
    }

    HALOperationResult Display_SSD1306::write(const HALWriteStringRequestValue& val) {
        ZeroCopyString zcData = val.value;
        printf("\nDisplay_SSD1306::write data:%s\n", zcData.ToString().c_str());
        ZeroCopyString zcCmd = zcData.SplitOffHead('/');
        
        if (zcCmd == "text") {
            printf("\nDisplay_SSD1306::write text:%s\n", zcData.ToString().c_str());
            display->write(zcData.start, zcData.Length());
        }
        else if (zcCmd == "print") {
            printf("\nDisplay_SSD1306::write print:%s\n", zcData.ToString().c_str());
            display->write(zcData.start, zcData.Length());
            display->display();
        }
        else if (zcCmd == "cursor") {
            ZeroCopyString zcXstr = zcData.SplitOffHead('/');
            int x = 0, y = 0;
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
        return HALOperationResult::Success;
    }

    void Display_SSD1306::loop() {
        display->clearDisplay();
        for (int i=0;i<elementCount;i++) {
            Display_SSD1306_Element& el = *elements[i];
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