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

#include "HearbeatLed.h"
#include <DALHAL/Support/ConvertHelper.h>
#include <DALHAL/Core/Types/DALHAL_ZeroCopyString.h>

namespace HeartbeatLed
{
    unsigned long HEARTBEATLED_ON_INTERVAL = HEARTBEATLED_DEFAULT_ON_INTERVAL;
    unsigned long HEARTBEATLED_OFF_INTERVAL = HEARTBEATLED_DEFAULT_OFF_INTERVAL;
    int ledState = LOW;             // ledState used to set the LED
    unsigned long previousMillis = 0;        // will store last time LED was updated
    unsigned long currentMillis = 0;
    unsigned long currentInterval = 0;
    
    bool parseCmd(DALHAL::ZeroCopyString& zcStr, DALHAL::StringBuilderStreamer& sbs)
    {
        DALHAL::ZeroCopyString zcCmd = zcStr.SplitOffHead('/');
        if (zcCmd.EqualsIC(F("set"))) {
            zcCmd = zcStr.SplitOffHead('/');
            DALHAL::ZeroCopyString zcValue = zcStr.SplitOffHead('/');
            if (zcValue.Length() == 0) {
                sbs.write(F("Error - 'set' cmd value missing: >>>")); sbs.write(zcCmd); sbs.write(F("<<<"));
                return false;
            }
            DALHAL::NumberResult numRes = zcValue.ConvertStringToNumber();
            if (numRes.type != DALHAL::NumberType::UINT32) {
                sbs.write(F("Error - 'set' cmd value is not a unsigned integer: >>>")); sbs.write(zcValue); sbs.write(F("<<<"));
                return false;
            }
            if (zcCmd.EqualsIC(F("on"))) {
                HeartbeatLed::HEARTBEATLED_ON_INTERVAL = numRes.u32;
            } else if (zcCmd.EqualsIC(F("off"))) {
                HeartbeatLed::HEARTBEATLED_OFF_INTERVAL = numRes.u32;
            } else {
                sbs.write(F("Error - 'set' cmd not found: >>>")); sbs.write(zcCmd); sbs.write(F("<<< (possible cmds: on, off)"));
                return false;
            }
            sbs.write(F("{\"OK\":\"value set\"}"));
            return true;
        } else {
            sbs.write(F("Error - cmd not found: >>>")); sbs.write(zcCmd); sbs.write(F("<<< (possible cmds: set)"));
            return false;
        }
        
    }
    void setup(unsigned long onInterval, unsigned long offInterval)
    {
        HEARTBEATLED_OFF_INTERVAL = offInterval;
        HEARTBEATLED_ON_INTERVAL = onInterval;
        setup();
    }
    void setup(void)
    {
        pinMode(HEARTBEATLED_PIN, OUTPUT); // output
        digitalWrite(HEARTBEATLED_PIN, HEARTBEATLED_INACTIVESTATE);
    }
    void task(void)
    {
        currentMillis = millis();
        currentInterval = currentMillis - previousMillis;
        
        if (ledState == LOW)
        {
            if (currentInterval > HEARTBEATLED_OFF_INTERVAL)
            {
                previousMillis = currentMillis;
                ledState = HIGH;
                digitalWrite(HEARTBEATLED_PIN, HEARTBEATLED_ACTIVESTATE);
            }
        }
        else
        {
            if (currentInterval > HEARTBEATLED_ON_INTERVAL)
            {
                previousMillis = currentMillis;
                ledState = LOW;
                digitalWrite(HEARTBEATLED_PIN, HEARTBEATLED_INACTIVESTATE);
            }
        }
    }
}