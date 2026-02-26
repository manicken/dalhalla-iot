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

#include "DALHAL_UID_Path.h"

#include "../../Support/ConvertHelper.h"
#include "../../Support/DALHAL_Logger.h"

namespace DALHAL {
    
    // static function
    bool UIDPath::Validate(const ZeroCopyString& zcStrUid) {
        if (zcStrUid.Length() == 0) {
            GlobalLogger.Error(F("UIDPath - is empty"));
            return false;
        }
        bool anyError = false;
        ZeroCopyString zcStrUidCpy = zcStrUid; // create copy
        while (zcStrUidCpy.Length())
        {
            ZeroCopyString zcTmp = zcStrUidCpy.SplitOffHead(':');
            if (zcTmp.Length() > HAL_UID::Size) {
                GlobalLogger.Error(F("UIDPath - uid too long"), zcTmp.ToString().c_str());
                anyError = true;
            }
        }
        return anyError == false;
    }

    UIDPath::UIDPath() = default;
    UIDPath::UIDPath(const char* cstr) : UIDPath(ZeroCopyString(cstr)) { }
    UIDPath::UIDPath(const std::string& uidStr) : UIDPath(ZeroCopyString(uidStr.c_str())) { }

    UIDPath::UIDPath(const ZeroCopyString& uidzcStr) {
        currentItemIndex = 0;
        if (uidzcStr.Length() == 0) {
            itemCount = 0; // allways used at reads so setting it to zero would make reads impossible
            GlobalLogger.Error(F("new UIDPath - input uidStr invalid"));
            return;
        }
        itemCount = uidzcStr.CountChar(':') + 1;
        items = new (std::nothrow) HAL_UID[itemCount];
        if (items == nullptr) {
            GlobalLogger.Error(F("new UIDPath - Allocation for items failed, count: "), std::to_string(itemCount).c_str());
            itemCount = 0; // allways used at reads so setting it to zero would make reads impossible
            return;
        }
        int index = 0;
        ZeroCopyString currZcStr = uidzcStr; // create copy
        while (currZcStr.Length())
        {
            items[index++] = encodeUID(currZcStr.SplitOffHead(':'));
        }
    }

    UIDPath::~UIDPath() {
        if (items != nullptr)
            delete[] items;
    }

    bool UIDPath::empty() const {
        return (!items || itemCount == 0);
    }
    
    uint32_t UIDPath::count() {
        return itemCount;
    }
    HAL_UID UIDPath::getCurrentUID() {
        if (currentItemIndex >= itemCount || items == nullptr) return HAL_UID::UID_INVALID; // ideally this wont happen
        return items[currentItemIndex];
    }
    HAL_UID UIDPath::getNextUID() {
        if (itemCount == 0 || items == nullptr) return HAL_UID::UID_INVALID; // ideally this wont happen
        if (currentItemIndex == (itemCount-1)) return HAL_UID::UID_INVALID; // ideally this wont happen
        currentItemIndex++;
        return items[currentItemIndex];
    }
    HAL_UID UIDPath::peekNextUID() {
        if (itemCount == 0 || items == nullptr) return HAL_UID::UID_INVALID; // ideally this wont happen
        if (currentItemIndex >= (itemCount-1)) return HAL_UID::UID_INVALID; // ideally this wont happen
        return items[currentItemIndex+1];
    }
    HAL_UID UIDPath::resetAndGetFirst() {
        if (itemCount == 0 || items == nullptr) return HAL_UID::UID_INVALID; // ideally this wont happen
        currentItemIndex = 0;
        return items[0];
    }
    void UIDPath::reset() {
        currentItemIndex = 0;
    }
    bool UIDPath::isLast() {
        if (itemCount == 0) return true; // ideally this wont happen
        return (currentItemIndex==(itemCount-1));
    }
    bool UIDPath::hasMore() {
        if (itemCount == 0) return false; // ideally this wont happen
        return (currentItemIndex<(itemCount-1));
    }

    std::string UIDPath::ToString(ToStringType type) {
        std::string ret;
        for (uint32_t i=0;i<itemCount;i++) {
            if (type == ToStringType::String) {
                ret += std::string(decodeUID(items[i]).c_str());
            } else if (type == ToStringType::Raw) {
                ret += Convert::toHex(items[i].val);
            }
            if (i<itemCount-1)
                ret += ":";
        }
        return ret;
    }
}