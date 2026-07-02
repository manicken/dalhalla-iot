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

#include <DALHAL/Core/Types/DALHAL_Device.h>
#include "DALHAL_Display_SSD1306_Element.h"

#include <DALHAL/Support/DALHAL_Logger.h>

#include "DALHAL_Display_SSD1306_JSON_Schema.h"

namespace DALHAL {

    constexpr Registry::Item SubItemTypeRegItems[] = {
        {"element", &Display_SSD1306_Element::RegistryDefine}
    };

    constexpr Registry::DeviceRegistry SubItemTypeReg = {
        SubItemTypeRegItems, sizeof(SubItemTypeRegItems)/sizeof(SubItemTypeRegItems[0]), "element", "ROOT:I2C_MASTER"
    };

    __attribute__((used, externally_visible))
    constexpr I2C_RegistryDefine Display_SSD1306::RegistryDefine = {
        &SubItemTypeReg,
        Create,
        &JsonSchema::Display_SSD1306::Root,
        DALHAL_REACTIVE_EVENT_TABLE(DISPLAY_SSD1306),
        &Display_SSD1306::FunctionTable,
        HasAddress,
    };

    /* override */
    const Registry::DefineBase* Display_SSD1306::GetRegistryDefine() {
        return &RegistryDefine;
    }

    constexpr FunctionEntry<FunctionTypes::WriteString> Display_SSD1306::writeStringFunctions[] = {
        DALHAL_FUNCTION_ENTRY("setCursor", setCursor, "sets the cursor"),
        DALHAL_FUNCTION_ENTRY("addText", addText, "add text to the buffer data"),
        DALHAL_FUNCTION_ENTRY("printText", printText, "add text and display the buffer data")
    };

    __attribute__((used, externally_visible))
    constexpr FunctionEntry<FunctionTypes::Exec> Display_SSD1306::execFunctions[] = {
        DALHAL_FUNCTION_ENTRY("update", display_update, "display the data in the buffer"),
        DALHAL_FUNCTION_ENTRY("clear", display_clear, "clear the display")
    };

    __attribute__((used, externally_visible))
    constexpr DeviceFunctionTable Display_SSD1306::FunctionTable = {
        EmptyFunctionTable<FunctionTypes::Exec>,
        EmptyFunctionTable<FunctionTypes::ReadToHALValue>, 
        EmptyFunctionTable<FunctionTypes::WriteHALValue>,
        EmptyFunctionTable<FunctionTypes::BracketOpRead>,
        EmptyFunctionTable<FunctionTypes::BracketOpWrite>,
        EmptyFunctionTable<FunctionTypes::ReadString>,
        DALHAL_FUNCTION_TABLE_ENTRY(writeStringFunctions)
    };

    bool Display_SSD1306::HasAddress(uint8_t addr) {
        return addr == 0x3C || addr == 0x3D; 
    }

    Device* Display_SSD1306::Create(DeviceCreateContext& context) {
        return new Display_SSD1306(static_cast<I2C_Master_CreateFunctionContext&>(context));
    }
    
    Display_SSD1306::Display_SSD1306(I2C_Master_CreateFunctionContext& context) : Display_SSD1306_DeviceBase(context.deviceType) {
        JsonSchema::Display_SSD1306::Extractors::Apply(context, this);
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
        // note to myself and others just ignore the warning this give
        // as it's because the class uses virtuals but the destructor is not virtual
        delete display;
        /* the following silence the warning but is very ugly
        if (display != nullptr) {
            Adafruit_SSD1306* adafruitDisplay = static_cast<Adafruit_SSD1306*>(display);
            adafruitDisplay->~Adafruit_SSD1306();
            ::operator delete(adafruitDisplay);
            display = nullptr;
        }*/
    }

    void Display_SSD1306::PrintTo(StringBuilderStreamer& sbs) {
        Device::PrintTo(sbs);
        sbs.write_json_value_separator();
        sbs.write_jsonNumber(F("width"), display->width());
        sbs.write_json_value_separator();
        sbs.write_jsonNumber(F("height"), display->height());

    }

    DeviceFindResult Display_SSD1306::findDevice(UIDPath& path, Device*& outDevice) {
        return Device::findInArray(elements, elementCount, path, this, outDevice);
    }

    /* static */
    HALOperationResult Display_SSD1306::display_update(Device* device) {
        static_cast<Display_SSD1306*>(device)->display->display();
        return HALOperationResult::Success;
    }

    /* static */
    HALOperationResult Display_SSD1306::display_clear(Device* device) {
        static_cast<Display_SSD1306*>(device)->display->clearDisplay();
        return HALOperationResult::Success;
    }

    /* static */
    HALOperationResult Display_SSD1306::setCursor(Device* device, const ZeroCopyString& zcParams, StringBuilderStreamer& sbs) {
        ZeroCopyString zcParamsTemp = zcParams;
        ZeroCopyString zcXstr = zcParamsTemp.SplitOffHead('/');
        int32_t x = 0, y = 0;
        zcXstr.ConvertTo_int32(x);
        zcParams.ConvertTo_int32(y);
        static_cast<Display_SSD1306*>(device)->display->setCursor(x,y);
        return HALOperationResult::Success;
    }
    /* static */
    HALOperationResult Display_SSD1306::addText(Device* device, const ZeroCopyString& zcParams, StringBuilderStreamer& sbs) {
        static_cast<Display_SSD1306*>(device)->display->write(zcParams.start, zcParams.Length());
        return HALOperationResult::Success;
    }
    /* static */
    HALOperationResult Display_SSD1306::printText(Device* device, const ZeroCopyString& zcParams, StringBuilderStreamer& sbs) {
        static_cast<Display_SSD1306*>(device)->display->write(zcParams.start, zcParams.Length());
        static_cast<Display_SSD1306*>(device)->display->display();
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
            // as we do actually check the halvalue type explicit
            // we can use the raw getters
            HALValue::Type t = el.val.getType();
            if (t == HALValue::Type::FLOAT) {
                display->print(el.val.asRawFloat());
            } else if (t == HALValue::Type::UINT) {
                display->print(el.val.asRawUInt());
            } else if (t == HALValue::Type::INT) {
                display->print(el.val.asRawInt());
            } else if (t == HALValue::Type::BOOL) {
                display->print(el.val.asRawBool());
            } else if (t == HALValue::Type::CSTRING) {
                display->print(el.val.toConstChar());
            }
        }
        display->display(); // update all in one go
    }
}